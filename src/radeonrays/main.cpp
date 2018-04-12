/**********************************************************************
Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
********************************************************************/
#define RR_STATIC_LIBRARY true
#include "radeon_rays.h"
#define USE_OPENCL true
#include "radeon_rays_cl.h"
#include "CLW.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cassert>
#include <iostream>
#include <memory>
#include <chrono>
#include <map>
#include <string>
#include <iomanip>
#include <sstream>

#include "shader_manager.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define MAX_BOUNCES 4

using namespace RadeonRays;
using namespace tinyobj;

namespace {
    std::vector<shape_t> g_objshapes;
    std::vector<material_t> g_objmaterials;

	unsigned char * texturePool = nullptr;
	size_t texturePoolSize = 0;

	struct Texture {
		uint32_t width;
		uint32_t height;
		size_t   offset;

		Texture(uint32_t width, uint32_t height, size_t offset) : width(width), height(height), offset(offset) {}
		Texture(void) : width(0), height(0), offset(0) {}
	};

    GLuint g_vertex_buffer, g_index_buffer;
    GLuint g_texture;
    std::unique_ptr<ShaderManager> g_shader_manager;
	GLint g_texelStepLocation;
	GLint g_fxaaOnLocation;

	GLint g_lumaThresholdLocation;
	GLint g_mulReduceLocation;
	GLint g_minReduceLocation;
	GLint g_maxSpanLocation;

	int g_fxaaOn = 1;

	float g_lumaThreshold = 0.5f;
	float g_mulReduceReciprocal = 8.0f;
	float g_minReduceReciprocal = 128.0f;
	float g_maxSpan = 8.0f;

    IntersectionApi* g_api;

    //CL data
    CLWContext g_context;
    CLWProgram g_program;
    CLWBuffer<float> g_positions;
    CLWBuffer<float> g_normals;
	CLWBuffer<float> g_texcoords;
	CLWBuffer<int> g_indices;
    CLWBuffer<float> g_ambient;
    CLWBuffer<float> g_diffuse;
	CLWBuffer<float> g_ior;
	CLWBuffer<int> g_indent;
	CLWBuffer<Texture> g_textures;
	CLWBuffer<unsigned char> g_texturePool;

    //Camera
    struct Camera
    {
        // Camera coordinate frame
        RadeonRays::float3 forward;
		RadeonRays::float3 up;
		RadeonRays::float3 right;
		RadeonRays::float3 p;

        // Near and far Z
        float2 zcap;

        // Speed
        float speed;

        // Direction
        float yaw;
        float pitch;
    } cam;

    // Point light position
	RadeonRays::float3 light = { -5.795126f, 0.277411f, 0.285406f };

    //Mouse struct for mouse handling
    struct MOUSE {
        bool pressed;
        int button;
        double xpos;
        double ypos;
    } mouse;

    float frameTime;
#define TIME_INTERVAL 1.0f

    std::string WINDOW_TITLE = "RadeonRays Test";
    GLFWmonitor* monitor;
    const GLFWvidmode* mode;
    GLFWwindow* window;
    int g_window_width = 800;
    int g_window_height = 600;
    int k_raypack_size = g_window_height * g_window_width;

    //RadeonRays buffers
    Buffer* ray_buffer;
    Buffer* shadow_rays_buffer;
    Buffer* isect_buffer;
    Buffer* occl_buffer;
    Buffer* tex_buf;

    //OpenCL buffers
    CLWBuffer<ray> ray_buffer_cl;
    CLWBuffer<ray> shadow_rays_buffer_cl;
    CLWBuffer<Intersection> isect_buffer_cl;
    CLWBuffer<Intersection> occl_buffer_cl;
    CLWBuffer<unsigned char> tex_buffer_cl;

    //OpenGL buffers
    GLuint g_vao;
    GLuint program;
    GLuint position_attr;
    GLuint texcoord_attr;
}

Texture loadTexture(const std::string &filename)
{
	Texture texture = {};

	if (!filename.empty()) {

		int width = 0;
		int height = 0;
		int num_components = 0;

		std::unique_ptr<unsigned char, void(*)(void*)> stbi_data(stbi_load(filename.c_str(), &width, &height, &num_components, 3), stbi_image_free);

		if ((!stbi_data) ||
			(0 >= width) ||
			(0 >= height) ||
			(3 != num_components)) {
			std::stringstream errorMessage;
			errorMessage << "could not read image " << filename << " !";
			throw std::runtime_error(errorMessage.str().c_str());
		}

		size_t data_size = static_cast<size_t>(width * height * 3 * sizeof(unsigned char));
		texture.width = static_cast<uint32_t>(width);
		texture.height = static_cast<uint32_t>(height);
		texture.offset = texturePoolSize;
		texturePoolSize += data_size;

		texturePool = (unsigned char *) std::realloc(texturePool, texturePoolSize);
		std::memcpy(texturePool + texture.offset, stbi_data.get(), data_size);
	}

	return texture;
}

void InitData(const char * filearg)
{
    std::string basepath = "data/";
	std::string filename;
	if (filearg == nullptr)
		filename = basepath + "sibenik.obj";
	else
		filename = basepath + filearg;
	std::string err;
    bool res = LoadObj(g_objshapes, g_objmaterials, err, filename.c_str(), basepath.c_str(), triangulation | calculate_normals);
	if (!err.empty())
		fprintf(stderr, "%s\n", err.c_str());

	if (!res)
		throw std::runtime_error("Loading obj file not successful");

	fprintf(stdout, "# of shapes    : %u\n", static_cast<uint32_t>(g_objshapes.size()));
	fprintf(stdout, "# of materials : %u\n", static_cast<uint32_t>(g_objmaterials.size()));

    // Load data to CL
	std::vector<float> verts = {};
	std::vector<float> normals = {};
	std::vector<float> texcoords = {};
	std::vector<int> inds = {};
	std::vector<float> ambient = {};
	std::vector<float> diffuse = {};
	std::vector<float> ior = {};
	std::vector<int> indents = {};
	std::vector<Texture> texture = {};
    int indent = 0;

	// find unique textures
	std::map<std::string, Texture> textureDiffuseMaps;
	for (auto &shape : g_objshapes)
	{
		const mesh_t& mesh = shape.mesh;
		for (int mat_id : mesh.material_ids)
		{
			const material_t& mat = g_objmaterials[mat_id];
			textureDiffuseMaps[mat.diffuse_texname.data()] = {};
		}
	}
	texturePool = (unsigned char *)malloc(0);
	for (auto &texture : textureDiffuseMaps) 
		texture.second = loadTexture(texture.first);

    for (auto &shape : g_objshapes)
    {
        const mesh_t& mesh = shape.mesh;
        verts.insert(verts.end(), mesh.positions.begin(), mesh.positions.end());
        normals.insert(normals.end(), mesh.normals.begin(), mesh.normals.end());
		texcoords.insert(texcoords.end(), mesh.texcoords.begin(), mesh.texcoords.end());
        inds.insert(inds.end(), mesh.indices.begin(), mesh.indices.end());
        for (int mat_id : mesh.material_ids)
        {
            const material_t& mat = g_objmaterials[mat_id];

            ambient.push_back(mat.ambient[0]);
            ambient.push_back(mat.ambient[1]);
            ambient.push_back(mat.ambient[2]);

            diffuse.push_back(mat.diffuse[0]);
            diffuse.push_back(mat.diffuse[1]);
            diffuse.push_back(mat.diffuse[2]);

			ior.push_back(mat.ior);

			texture.push_back(textureDiffuseMaps[mat.diffuse_texname.data()]);
        }

        // add additional empty data to simplify indentation in arrays
        if (mesh.positions.size() / 3 < mesh.indices.size())
        {
            int count = static_cast<int>(mesh.indices.size() - mesh.positions.size() / 3);
            for (int i = 0; i < count; ++i)
            {
				verts.push_back(0.f); normals.push_back(0.f); texcoords.push_back(0.f);
                verts.push_back(0.f); normals.push_back(0.f); texcoords.push_back(0.f);
                verts.push_back(0.f); normals.push_back(0.f);
            }
        }

        indents.push_back(indent);
        indent += static_cast<int>(mesh.indices.size());
    }

    g_positions = CLWBuffer<float>::Create(g_context, CL_MEM_READ_ONLY, verts.size(), verts.data());
    g_normals = CLWBuffer<float>::Create(g_context, CL_MEM_READ_ONLY, normals.size(), normals.data());
	g_texcoords = CLWBuffer<float>::Create(g_context, CL_MEM_READ_ONLY, texcoords.size(), texcoords.data());
	g_indices = CLWBuffer<int>::Create(g_context, CL_MEM_READ_ONLY, inds.size(), inds.data());
    g_ambient = CLWBuffer<float>::Create(g_context, CL_MEM_READ_ONLY, ambient.size(), ambient.data());
    g_diffuse = CLWBuffer<float>::Create(g_context, CL_MEM_READ_ONLY, diffuse.size(), diffuse.data());
	g_ior = CLWBuffer<float>::Create(g_context, CL_MEM_READ_ONLY, ior.size(), ior.data());
	g_indent = CLWBuffer<int>::Create(g_context, CL_MEM_READ_ONLY, indents.size(), indents.data());
	g_textures = CLWBuffer<Texture>::Create(g_context, CL_MEM_READ_ONLY, texture.size(), texture.data());
	g_texturePool = CLWBuffer<unsigned char>::Create(g_context, CL_MEM_READ_ONLY, texturePoolSize, texturePool);
}

void InitCl()
{
    std::vector<CLWPlatform> platforms;
    CLWPlatform::CreateAllPlatforms(platforms);

    if (platforms.size() == 0)
    {
        throw std::runtime_error("No OpenCL platforms installed.");
    }

    for (int i = 0; i < platforms.size(); ++i)
    {
        for (int d = 0; d < (int)platforms[i].GetDeviceCount(); ++d)
        {
            if (platforms[i].GetDevice(d).GetType() != CL_DEVICE_TYPE_GPU)
                continue;
            g_context = CLWContext::Create(platforms[i].GetDevice(d));
			fprintf(stdout, "Status: Using %s with %s\n", platforms[i].GetName().c_str(), platforms[i].GetDevice(d).GetName().c_str());
            break;
        }

        if (g_context)
            break;
    }
    const char* kBuildopts(" -cl-mad-enable -cl-fast-relaxed-math -cl-std=CL1.2 -I . ");

    g_program = CLWProgram::CreateFromFile("kernel.cl", kBuildopts, g_context);
}

Buffer* GeneratePrimaryRays()
{
    //pass data to buffers
    cl_float4 cam_pos_cl = { cam.p.x,
        cam.p.y,
        cam.p.z,
        cam.p.w };

    cl_float4 cam_forward_cl = { cam.forward.x,
        cam.forward.y,
        cam.forward.z,
        cam.forward.w };

    cl_float4 cam_right_cl = { cam.right.x,
        cam.right.y,
        cam.right.z,
        cam.right.w };

    cl_float4 cam_up_cl = { cam.up.x,
        cam.up.y,
        cam.up.z,
        cam.up.w };

    cl_float4 cam_zcap_cl = { cam.zcap.x,
        cam.zcap.y,
        0.0f,
        0.0f };

    //run kernel
    CLWKernel kernel = g_program.GetKernel("GeneratePerspectiveRays");
    kernel.SetArg(0, ray_buffer_cl);
    kernel.SetArg(1, cam_pos_cl);
    kernel.SetArg(2, cam_forward_cl);
    kernel.SetArg(3, cam_right_cl);
    kernel.SetArg(4, cam_up_cl);
    kernel.SetArg(5, cam_zcap_cl);
    kernel.SetArg(6, g_window_width);
    kernel.SetArg(7, g_window_height);

    // Run generation kernel
    size_t gs[] = { static_cast<size_t>((g_window_width + 7) / 8 * 8), static_cast<size_t>((g_window_height + 7) / 8 * 8) };
    size_t ls[] = { 8, 8 };
    g_context.Launch2D(0, gs, ls, kernel);
    g_context.Flush(0);

    return CreateFromOpenClBuffer(g_api, ray_buffer_cl);
}

Buffer* GenerateSecondaryRays(const CLWBuffer<Intersection> &isect)
{
	//run kernel
	CLWKernel kernel = g_program.GetKernel("GenerateSecondaryRays");
	kernel.SetArg(0, ray_buffer_cl);
	kernel.SetArg(1, g_normals);
	kernel.SetArg(2, g_indices);
	kernel.SetArg(3, g_indent);
	kernel.SetArg(4, g_ior);
	kernel.SetArg(5, isect);
	kernel.SetArg(6, g_window_width);
	kernel.SetArg(7, g_window_height);

	// Run generation kernel
	size_t gs[] = { static_cast<size_t>((g_window_width + 7) / 8 * 8), static_cast<size_t>((g_window_height + 7) / 8 * 8) };
	size_t ls[] = { 8, 8 };
	g_context.Launch2D(0, gs, ls, kernel);
	g_context.Flush(0);

	return CreateFromOpenClBuffer(g_api, ray_buffer_cl);
}

Buffer* GenerateShadowRays(CLWBuffer<Intersection> & isect, const RadeonRays::float3& light)
{
    //pass data to buffers
    cl_float4 light_cl = { light.x,
        light.y,
        light.z,
        light.w };

    //run kernel
    CLWKernel kernel = g_program.GetKernel("GenerateShadowRays");
    kernel.SetArg(0, shadow_rays_buffer_cl);
    kernel.SetArg(1, g_positions);
    kernel.SetArg(2, g_normals);
    kernel.SetArg(3, g_indices);
    kernel.SetArg(4, g_indent);
    kernel.SetArg(5, isect);
    kernel.SetArg(6, light_cl);
    kernel.SetArg(7, g_window_width);
    kernel.SetArg(8, g_window_height);

    // Run generation kernel
    size_t gs[] = { static_cast<size_t>((g_window_width + 7) / 8 * 8), static_cast<size_t>((g_window_height + 7) / 8 * 8) };
    size_t ls[] = { 8, 8 };
    g_context.Launch2D(0, gs, ls, kernel);
    g_context.Flush(0);

    return CreateFromOpenClBuffer(g_api, shadow_rays_buffer_cl);
}

Buffer* GenerateSecondaryShadowRays(CLWBuffer<Intersection> & occluds)
{
	//run kernel
	CLWKernel kernel = g_program.GetKernel("GenerateSecondaryShadowRays");
	kernel.SetArg(0, shadow_rays_buffer_cl);
	kernel.SetArg(1, g_indent);
	kernel.SetArg(2, g_ior);
	kernel.SetArg(3, occluds);
	kernel.SetArg(4, g_window_width);
	kernel.SetArg(5, g_window_height);

	// Run generation kernel
	size_t gs[] = { static_cast<size_t>((g_window_width + 7) / 8 * 8), static_cast<size_t>((g_window_height + 7) / 8 * 8) };
	size_t ls[] = { 8, 8 };
	g_context.Launch2D(0, gs, ls, kernel);
	g_context.Flush(0);

	return CreateFromOpenClBuffer(g_api, shadow_rays_buffer_cl);
}

Buffer* Shading(const CLWBuffer<Intersection> &isect, const CLWBuffer<Intersection> &occluds, const RadeonRays::float3& light)
{
    //pass data to buffers
    cl_float4 light_cl = { light.x,
        light.y,
        light.z,
        light.w };

    //run kernel
    CLWKernel kernel = g_program.GetKernel("Shading");
    kernel.SetArg(0, g_positions);
    kernel.SetArg(1, g_normals);
	kernel.SetArg(2, g_texcoords);
	kernel.SetArg(3, g_indices);
    kernel.SetArg(4, g_ambient);
    kernel.SetArg(5, g_diffuse);
	kernel.SetArg(6, g_texturePool);
    kernel.SetArg(7, g_indent);
	kernel.SetArg(8, g_textures);
	kernel.SetArg(9, isect);
    kernel.SetArg(10, occluds);
    kernel.SetArg(11, light_cl);
    kernel.SetArg(12, g_window_width);
    kernel.SetArg(13, g_window_height);
    kernel.SetArg(14, tex_buffer_cl);

    // Run generation kernel
    size_t gs[] = { static_cast<size_t>((g_window_width + 7) / 8 * 8), static_cast<size_t>((g_window_height + 7) / 8 * 8) };
    size_t ls[] = { 8, 8 };
    g_context.Launch2D(0, gs, ls, kernel);
    g_context.Flush(0);

    return CreateFromOpenClBuffer(g_api, tex_buffer_cl);
}

void InitGl()
{
    //Shader for the final full-screen quad
    g_shader_manager.reset(new ShaderManager());
    program = g_shader_manager->GetProgram("simple");
    GLuint texloc = glGetUniformLocation(program, "g_Texture");
    assert(texloc >= 0);
    glUniform1i(texloc, 0);
    position_attr = glGetAttribLocation(program, "inPosition");
    texcoord_attr = glGetAttribLocation(program, "inTexcoord");
	g_texelStepLocation = glGetUniformLocation(program, "u_texelStep");
	g_fxaaOnLocation = glGetUniformLocation(program, "u_fxaaOn");

	g_lumaThresholdLocation = glGetUniformLocation(program, "u_lumaThreshold");
	g_mulReduceLocation = glGetUniformLocation(program, "u_mulReduce");
	g_minReduceLocation = glGetUniformLocation(program, "u_minReduce");
	g_maxSpanLocation = glGetUniformLocation(program, "u_maxSpan");

	glUseProgram(program);
	glUniform2f(g_texelStepLocation, 1.0f / (float)g_window_width, 1.0f / (float)g_window_height);

    glGenVertexArrays(1, &g_vao);
    glGenBuffers(1, &g_vertex_buffer);
    glGenBuffers(1, &g_index_buffer);

    // create Vertex buffer
    glBindVertexArray(g_vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_vertex_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_index_buffer);
    glVertexAttribPointer(position_attr, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, 0);
    glVertexAttribPointer(texcoord_attr, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 3));
    glEnableVertexAttribArray(position_attr);
    glEnableVertexAttribArray(texcoord_attr);

    float quad_vdata[] =
    {
        -1, -1, 0.5, 0, 0,
        1, -1, 0.5, 1, 0,
        1, 1, 0.5, 1, 1,
        -1, 1, 0.5, 0, 1
    };

    GLshort quad_idata[] =
    {
        0, 1, 3,
        3, 1, 2
    };

    // fill data
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vdata), quad_vdata, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quad_idata), quad_idata, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Texture
    glGenTextures(1, &g_texture);
    glBindTexture(GL_TEXTURE_2D, g_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, g_window_width, g_window_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Initialize state
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glCullFace(GL_NONE);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, g_window_width, g_window_height);
}

void DrawScene(float time)
{
    static uint32_t frame = 0;
    static float total_time = 0.0f;

    //static auto startTime = std::chrono::high_resolution_clock::now();
    auto startFrameTime = std::chrono::high_resolution_clock::now();

    //float time = std::chrono::duration_cast<std::chrono::microseconds>(startFrameTime - startTime).count() / 1e6f;

    // Process mouse input, if any
    if (mouse.pressed) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        switch (mouse.button) {
            case(GLFW_MOUSE_BUTTON_LEFT):
                cam.yaw   += (float)(xpos - mouse.xpos) * 0.005f;
                cam.pitch += (float)(ypos - mouse.ypos) * 0.005f;

                cam.forward.x = sinf(cam.yaw) * cosf(cam.pitch);
                cam.forward.y = sinf(cam.pitch);
                cam.forward.z = cosf(cam.yaw) * cosf(cam.pitch);
                cam.forward.normalize();

                cam.right = cross(cam.forward, RadeonRays::float3(0.0f, 1.0f, 0.0f));
                cam.right.normalize();

                cam.up = cross(cam.right, cam.forward);
                cam.up.normalize();

                break;
            case(GLFW_MOUSE_BUTTON_MIDDLE):
                light += (float)(xpos - mouse.xpos) * 0.005f * cam.right +
						 (float)(ypos - mouse.ypos) * 0.005f * cam.forward;
                break;
			case(GLFW_MOUSE_BUTTON_RIGHT):
				light -= (float)(ypos - mouse.ypos) * 0.005f * cam.up;
				break;
		}
        mouse.xpos = xpos;
        mouse.ypos = ypos;
    }

    // Generate primary rays
    ray_buffer = GeneratePrimaryRays();

    // Intersection
    g_api->QueryIntersection(ray_buffer, k_raypack_size, isect_buffer, nullptr, nullptr);

	for (uint32_t bounce = 0; bounce < MAX_BOUNCES; ++bounce) {
		// Generate secondary rays
		ray_buffer = GenerateSecondaryRays(isect_buffer_cl);

		// Intersection again
		g_api->QueryIntersection(ray_buffer, k_raypack_size, isect_buffer, nullptr, nullptr);
	}

    // Generate shadow rays
    shadow_rays_buffer = GenerateShadowRays(isect_buffer_cl, light);

    // Intersection
    g_api->QueryIntersection(shadow_rays_buffer, k_raypack_size, occl_buffer, nullptr, nullptr);

	for (uint32_t bounce = 0; bounce < MAX_BOUNCES; ++bounce) {
		// Generate secondary shadow rays
		shadow_rays_buffer = GenerateSecondaryShadowRays(occl_buffer_cl);

		// Intersection again
		g_api->QueryIntersection(shadow_rays_buffer, k_raypack_size, occl_buffer, nullptr, nullptr);
	}

    // Shading
    tex_buf = Shading(isect_buffer_cl, occl_buffer_cl, light);

    // Get image data
    std::vector<unsigned char> tex_data(k_raypack_size * 4);
    unsigned char* pixels = nullptr;
    Event* e = nullptr;
    g_api->MapBuffer(tex_buf, kMapRead, 0, 4 * k_raypack_size * sizeof(unsigned char), (void**)&pixels, &e);
    e->Wait();
    memcpy(tex_data.data(), pixels, 4 * k_raypack_size * sizeof(unsigned char));

    // Update texture data
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, g_window_width, g_window_height, GL_RGBA, GL_UNSIGNED_BYTE, tex_data.data());

    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(g_vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_vertex_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_index_buffer);

    // shader data
    glUseProgram(program);
	glUniform1i(g_fxaaOnLocation, g_fxaaOn);

	glUniform1f(g_lumaThresholdLocation, g_lumaThreshold);
	glUniform1f(g_mulReduceLocation, 1.0f / g_mulReduceReciprocal);
	glUniform1f(g_minReduceLocation, 1.0f / g_minReduceReciprocal);
	glUniform1f(g_maxSpanLocation, g_maxSpan);

    // draw rectangle
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    auto endFrameTime = std::chrono::high_resolution_clock::now();
    frameTime = std::chrono::duration_cast<std::chrono::microseconds>(endFrameTime - startFrameTime).count() / 1e6f;
    total_time += frameTime;
    frame++;

    if (total_time >= TIME_INTERVAL) {
        std::stringstream stream;
        stream << WINDOW_TITLE << " - " << std::fixed << std::setprecision(0) << (float)frame / total_time << " fps";
        glfwSetWindowTitle(window, stream.str().c_str());
        frame = 0;
        total_time = 0.0f;
    }
}

static void onKey(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        case 'W':
            cam.p += cam.speed * cam.forward * frameTime;
            break;
        case 'S':
            cam.p -= cam.speed * cam.forward * frameTime;
            break;
        case 'D':
            cam.p += cam.speed * cam.right * frameTime;
            break;
        case 'A':
            cam.p -= cam.speed * cam.right * frameTime;
            break;
        case 'Q':
            cam.p += cam.speed * cam.up * frameTime;
            break;
        case 'Z':
            cam.p -= cam.speed * cam.up * frameTime;
            break;
		case 'P':
			fprintf(stdout, "cam.p       = { %ff, %ff, %ff };\n", cam.p.x, cam.p.y, cam.p.z);
			fprintf(stdout, "cam.forward = { %ff, %ff, %ff };\n", cam.forward.x, cam.forward.y, cam.forward.z);
			fprintf(stdout, "cam.right   = { %ff, %ff, %ff };\n", cam.right.x, cam.right.y, cam.right.z);
			fprintf(stdout, "cam.up      = { %ff, %ff, %ff };\n", cam.up.x, cam.up.y, cam.up.z);
			fprintf(stdout, "cam.pitch = %ff;\n", cam.pitch);
			fprintf(stdout, "cam.yaw   = %ff;\n", cam.yaw);
			break;
		case 'F':
			g_fxaaOn = !g_fxaaOn;
			break;
		case '1':
			g_lumaThreshold -= 0.05f;
			break;
		case '2':
			g_lumaThreshold += 0.05f;
			break;
		case '3':
			g_mulReduceReciprocal *= 2.0f;
			break;
		case '4':
			g_mulReduceReciprocal /= 2.0f;
			break;
		case '5':
			g_minReduceReciprocal *= 2.0f;
			break;
		case '6':
			g_minReduceReciprocal /= 2.0f;
			break;
		case '7':
			g_maxSpan -= 1.0f;
			break;
		case '8':
			g_maxSpan += 1.0f;
			break;
        }
    }
}

void onMouseButton(GLFWwindow *window, int button, int action, int mods)
{
    mouse.button = button;
    mouse.pressed = !mouse.pressed;
    glfwGetCursorPos(window, &mouse.xpos, &mouse.ypos);
}

int main(int argc, char* argv[])
{
    // GLFW Window Initialization:
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }
    fprintf(stdout, "Status: Using GLFW %s\n", glfwGetVersionString());

    monitor = glfwGetPrimaryMonitor();
    mode = glfwGetVideoMode(monitor);

    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_REFRESH_RATE, GLFW_DONT_CARE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 0);
    window = glfwCreateWindow(g_window_width, g_window_height, WINDOW_TITLE.c_str(), NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to open window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);
    glfwSetKeyCallback(window, onKey);
    glfwSetMouseButtonCallback(window, onMouseButton);
    //glfwSetCursorPosCallback(window, mouseMove_callback);
    //glfwSetScrollCallback(window, mouseWheel_callback);
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));

    // Prepare rectangle for drawing texture
    // rendered using intersection results
	try {

    InitGl();

    InitCl();

    // Load CornellBox model
    InitData(argv[1]);

    // Create api using already existing opencl context
    cl_device_id id = g_context.GetDevice(0).GetID();
    cl_command_queue queue = g_context.GetCommandQueue(0);

    // Create intersection API
    g_api = RadeonRays::CreateFromOpenClContext(g_context, id, queue);

    // Adding meshes to tracing scene
    for (int id = 0; id < g_objshapes.size(); ++id)
    {
        shape_t& objshape = g_objshapes[id];
        float* vertdata = objshape.mesh.positions.data();
        int nvert = static_cast<int>(objshape.mesh.positions.size() / 3);
        int* indices = reinterpret_cast<int *>(objshape.mesh.indices.data());
        int nfaces = static_cast<int>(objshape.mesh.indices.size() / 3);
        Shape* shape = g_api->CreateMesh(vertdata, nvert, 3 * sizeof(float), indices, 0, nullptr, nfaces);

        assert(shape != nullptr);
        g_api->AttachShape(shape);
        shape->SetId(id);
    }
    // Commit scene changes
    g_api->Commit();

    // Setup camera
	cam.p = { 0.516821f, -12.180884f, 0.081849f };
	cam.forward = { 0.908958f, 0.416871f, 0.003823f };
    cam.up = { -0.416867f, 0.908966f, -0.001753f };
    cam.right = { -0.004205f, 0.000000f, 0.999991f };
	cam.pitch = 0.43f;
	cam.yaw = 1.566591f;
	cam.zcap = { 1.f, 1000.f };
    cam.speed = 10.0f;

    // Create OpenCL buffers
    ray_buffer_cl         = CLWBuffer<ray>::Create(g_context, CL_MEM_READ_WRITE, k_raypack_size);
    shadow_rays_buffer_cl = CLWBuffer<ray>::Create(g_context, CL_MEM_READ_WRITE, k_raypack_size);
    isect_buffer_cl       = CLWBuffer<Intersection>::Create(g_context, CL_MEM_READ_WRITE, k_raypack_size);
    occl_buffer_cl        = CLWBuffer<Intersection>::Create(g_context, CL_MEM_READ_WRITE, k_raypack_size);
    tex_buffer_cl         = CLWBuffer<unsigned char>::Create(g_context, CL_MEM_READ_ONLY, 4 * k_raypack_size);

    // Create the intersection and occlusion buffers
    isect_buffer = CreateFromOpenClBuffer(g_api, isect_buffer_cl);
    occl_buffer = CreateFromOpenClBuffer(g_api, occl_buffer_cl);

    // Start the main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        DrawScene(static_cast<float>(glfwGetTime()));
        glfwSwapBuffers(window);
    }

	}
	catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

    // Cleanup
    IntersectionApi::Delete(g_api); g_api = nullptr;
	free(texturePool);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
