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
#include <string>
#include <iomanip>
#include <sstream>
#include "shader_manager.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

using namespace RadeonRays;
using namespace tinyobj;

namespace {
    std::vector<shape_t> g_objshapes;
    std::vector<material_t> g_objmaterials;

    GLuint g_vertex_buffer, g_index_buffer;
    GLuint g_texture;
    std::unique_ptr<ShaderManager> g_shader_manager;

    IntersectionApi* g_api;

    //CL data
    CLWContext g_context;
    CLWProgram g_program;
    CLWBuffer<float> g_positions;
    CLWBuffer<float> g_normals;
    CLWBuffer<int> g_indices;
    CLWBuffer<float> g_ambient;
    CLWBuffer<float> g_diffuse;
    CLWBuffer<int> g_indent;

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
	RadeonRays::float3 light = { -0.01f, 1.85f, 0.1f };

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
    int g_window_width = 640;
    int g_window_height = 480;
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
    CLWBuffer<int> occl_buffer_cl;
    CLWBuffer<unsigned char> tex_buffer_cl;

    //OpenGL buffers
    GLuint g_vao;
    GLuint program;
    GLuint position_attr;
    GLuint texcoord_attr;
}

void InitData()
{
    std::string basepath = "data/";
    std::string filename = basepath + "orig.objm";
	std::string err;
    bool res = LoadObj(g_objshapes, g_objmaterials, err, filename.c_str(), basepath.c_str(), triangulation | calculate_normals);
	if (!err.empty())
		fprintf(stderr, "%s\n", err.c_str());

	if (!res)
		throw std::runtime_error("Loading obj file not succesfull");

	fprintf(stdout, "# of shapes    : %u\n", static_cast<uint32_t>(g_objshapes.size()));
	fprintf(stdout, "# of materials : %u\n", static_cast<uint32_t>(g_objmaterials.size()));

    // Load data to CL
    std::vector<float> verts;
    std::vector<float> normals;
    std::vector<int> inds;
    std::vector<float> ambient;
    std::vector<float> diffuse;
    std::vector<int> indents;
    int indent = 0;

    for (auto &shape : g_objshapes)
    {
        const mesh_t& mesh = shape.mesh;
        verts.insert(verts.end(), mesh.positions.begin(), mesh.positions.end());
        normals.insert(normals.end(), mesh.normals.begin(), mesh.normals.end());
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
        }

        // add additional empty data to simplify indentation in arrays
        if (mesh.positions.size() / 3 < mesh.indices.size())
        {
            int count = static_cast<int>(mesh.indices.size() - mesh.positions.size() / 3);
            for (int i = 0; i < count; ++i)
            {
                verts.push_back(0.f); normals.push_back(0.f);
                verts.push_back(0.f); normals.push_back(0.f);
                verts.push_back(0.f); normals.push_back(0.f);
            }
        }

        indents.push_back(indent);
        indent += static_cast<int>(mesh.indices.size());
    }

    g_positions = CLWBuffer<float>::Create(g_context, CL_MEM_READ_ONLY, verts.size(), verts.data());
    g_normals = CLWBuffer<float>::Create(g_context, CL_MEM_READ_ONLY, normals.size(), normals.data());
    g_indices = CLWBuffer<int>::Create(g_context, CL_MEM_READ_ONLY, inds.size(), inds.data());
    g_ambient = CLWBuffer<float>::Create(g_context, CL_MEM_READ_ONLY, ambient.size(), ambient.data());
    g_diffuse = CLWBuffer<float>::Create(g_context, CL_MEM_READ_ONLY, diffuse.size(), diffuse.data());
    g_indent = CLWBuffer<int>::Create(g_context, CL_MEM_READ_ONLY, indents.size(), indents.data());
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
    kernel.SetArg(4, g_ambient);
    kernel.SetArg(5, g_diffuse);
    kernel.SetArg(6, g_indent);
    kernel.SetArg(7, isect);
    kernel.SetArg(8, light_cl);
    kernel.SetArg(9, g_window_width);
    kernel.SetArg(10, g_window_height);

    // Run generation kernel
    size_t gs[] = { static_cast<size_t>((g_window_width + 7) / 8 * 8), static_cast<size_t>((g_window_height + 7) / 8 * 8) };
    size_t ls[] = { 8, 8 };
    g_context.Launch2D(0, gs, ls, kernel);
    g_context.Flush(0);

    return CreateFromOpenClBuffer(g_api, shadow_rays_buffer_cl);
}

Buffer* Shading(const CLWBuffer<Intersection> &isect, const CLWBuffer<int> &occluds, const RadeonRays::float3& light)
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
    kernel.SetArg(2, g_indices);
    kernel.SetArg(3, g_ambient);
    kernel.SetArg(4, g_diffuse);
    kernel.SetArg(5, g_indent);
    kernel.SetArg(6, isect);
    kernel.SetArg(7, occluds);
    kernel.SetArg(8, light_cl);
    kernel.SetArg(9, g_window_width);
    kernel.SetArg(10, g_window_height);
    kernel.SetArg(11, tex_buffer_cl);

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
                light.x += (float)(xpos - mouse.xpos) * 0.005f;
                light.z += (float)(ypos - mouse.ypos) * 0.005f;
                break;
        }
        mouse.xpos = xpos;
        mouse.ypos = ypos;
    }

    // Generate primary rays
    ray_buffer = GeneratePrimaryRays();

    // Intersection
    g_api->QueryIntersection(ray_buffer, k_raypack_size, isect_buffer, nullptr, nullptr);

    // Generate shadow rays
    shadow_rays_buffer = GenerateShadowRays(isect_buffer_cl, light);

    // Occlusion
    g_api->QueryOcclusion(shadow_rays_buffer, k_raypack_size, occl_buffer, nullptr, nullptr);

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
        }
    }
}

void onWindowSize(GLFWwindow * window, int width, int height)
{/*
    g_window_width = width;
    g_window_height = height;
    k_raypack_size = g_window_width * g_window_height;
    glViewport(0, 0, g_window_width, g_window_height);

    delete ray_buffer_cl;
    delete shadow_rays_buffer_cl;
    delete isect_buffer_cl;
    delete occl_buffer_cl;
    delete tex_buffer_cl;

    ray_buffer_cl = CLWBuffer<ray>::Create(g_context, CL_MEM_READ_WRITE, k_raypack_size);
    shadow_rays_buffer_cl = CLWBuffer<ray>::Create(g_context, CL_MEM_READ_WRITE, k_raypack_size);
    isect_buffer_cl = CLWBuffer<Intersection>::Create(g_context, CL_MEM_READ_WRITE, k_raypack_size);
    occl_buffer_cl = CLWBuffer<int>::Create(g_context, CL_MEM_READ_WRITE, k_raypack_size);
    tex_buffer_cl = CLWBuffer<unsigned char>::Create(g_context, CL_MEM_READ_ONLY, 4 * k_raypack_size);

    isect_buffer = CreateFromOpenClBuffer(g_api, isect_buffer_cl);
    occl_buffer = CreateFromOpenClBuffer(g_api, occl_buffer_cl);
    */
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
    glfwSetWindowSizeCallback(window, onWindowSize);
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
    InitGl();

    InitCl();

    // Load CornellBox model
    InitData();

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
        int nvert = static_cast<int>(objshape.mesh.positions.size());
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
    cam.forward = { 0.f, 0.f, -1.f };
    cam.up = { 0.f, 1.f, 0.f };
    cam.right = { 1.f, 0.f, 0.f };
    cam.p = { 0.f, 1.f, 2.f };
    cam.zcap = { 1.f, 1000.f };
    cam.speed = 10.0f;
    cam.yaw = 3.14159f;
    cam.pitch = 0.0f;

    // Create OpenCL buffers
    ray_buffer_cl         = CLWBuffer<ray>::Create(g_context, CL_MEM_READ_WRITE, k_raypack_size);
    shadow_rays_buffer_cl = CLWBuffer<ray>::Create(g_context, CL_MEM_READ_WRITE, k_raypack_size);
    isect_buffer_cl       = CLWBuffer<Intersection>::Create(g_context, CL_MEM_READ_WRITE, k_raypack_size);
    occl_buffer_cl        = CLWBuffer<int>::Create(g_context, CL_MEM_READ_WRITE, k_raypack_size);
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

    // Cleanup
    IntersectionApi::Delete(g_api); g_api = nullptr;
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
