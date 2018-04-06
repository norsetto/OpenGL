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
#ifndef KERNEL_CL
#define KERNEL_CL

#define EPSILON 0.001f

typedef struct _Ray
{
    /// xyz - origin, w - max range
    float4 o;
    /// xyz - direction, w - time
    float4 d;
    /// x - ray mask, y - activity flag
    int2 extra;
    /// Padding
    float2 padding;
} Ray;

typedef struct _Intersection
{
    // id of a shape
    int shapeid;
    // Primitive index
    int primid;
    // Padding elements
    int padding0;
    int padding1;

    // uv - hit barycentrics, w - ray distance
    float4 uvwt;
} Intersection;

typedef struct _Texture
{
    // Width, height
    unsigned int w;
    unsigned int h;
    // Offset in texture data array
    unsigned long int offset;
} Texture;

float4 ConvertFromBarycentric3(__global const float* vec,
                               __global const int* ind,
                               int prim_id,
                               const float4 uvwt)
{
    float4 a = (float4)(vec[ind[prim_id * 3 + 0] * 3 + 0],
                        vec[ind[prim_id * 3 + 0] * 3 + 1],
                        vec[ind[prim_id * 3 + 0] * 3 + 2], 0.f);

    float4 b = (float4)(vec[ind[prim_id * 3 + 1] * 3 + 0],
                        vec[ind[prim_id * 3 + 1] * 3 + 1],
                        vec[ind[prim_id * 3 + 1] * 3 + 2], 0.f);

    float4 c = (float4)(vec[ind[prim_id * 3 + 2] * 3 + 0],
                        vec[ind[prim_id * 3 + 2] * 3 + 1],
                        vec[ind[prim_id * 3 + 2] * 3 + 2], 0.f);

    return a * (1 - uvwt.x - uvwt.y) + b * uvwt.x + c * uvwt.y;
}

float2 ConvertFromBarycentric2(__global const float* vec,
                               __global const int* ind,
                               int prim_id,
                               const float4 uvwt)
{
    float2 a = (float2)(vec[ind[prim_id * 3 + 0] * 2 + 0],
                        vec[ind[prim_id * 3 + 0] * 2 + 1]);

    float2 b = (float2)(vec[ind[prim_id * 3 + 1] * 2 + 0],
                        vec[ind[prim_id * 3 + 1] * 2 + 1]);

    float2 c = (float2)(vec[ind[prim_id * 3 + 2] * 2 + 0],
                        vec[ind[prim_id * 3 + 2] * 2 + 1]);

    return a * (1 - uvwt.x - uvwt.y) + b * uvwt.x + c * uvwt.y;
}

__kernel void GeneratePerspectiveRays(__global Ray* rays,
                                    const float4 cam_pos,
                                    const float4 cam_forward,
                                    const float4 cam_right,
                                    const float4 cam_up,
                                    const float4 cam_zcap,
                                    int width,
                                    int height)
{
    int2 globalid;
    globalid.x  = get_global_id(0);
    globalid.y  = get_global_id(1);

    // Check borders
    if (globalid.x < width && globalid.y < height)
    {
        //pixel coordinates on camera plane
        float x = 2.0 * ((float)globalid.x / (float)width  - 0.5);
        float y = 2.0 * ((float)globalid.y / (float)height - 0.5);

        // Perspective view
        int k = globalid.y * width + globalid.x;

        rays[k].d.xyz = normalize(cam_zcap.x * cam_forward.xyz + x * cam_right.xyz + y * cam_up.xyz);
        rays[k].d.w = 0.0;
        rays[k].o.xyz = cam_pos.xyz + cam_zcap.x * rays[k].d.xyz;
        rays[k].o.w = cam_zcap.y - cam_zcap.x;

        rays[k].extra.x = 0xFFFFFFFF;
        rays[k].extra.y = 0xFFFFFFFF;
    }
}

__kernel void GenerateShadowRays(__global Ray* rays,
                            //scene
                            __global float* positions,
                            __global float* normals,
                            __global int* ids,
                            __global int* indents,
                            //intersection
                            __global Intersection* isect,
                            //light pos
                            float4 light,
                            //window size
                            int width,
                            int height)
{
    int2 globalid;
    globalid.x  = get_global_id(0);
    globalid.y  = get_global_id(1);

    // Check borders
    if (globalid.x < width && globalid.y < height)
    {
        int k = globalid.y * width + globalid.x;
        int shape_id = isect[k].shapeid;
        int prim_id = isect[k].primid;

        // Need shadow rays only for intersections
        if (shape_id == -1 || prim_id == -1)
        {
           return;
        }

        // Calculate position and normal of the intersection point
        int ind = indents[shape_id];
        float4 pos = ConvertFromBarycentric3(positions + ind*3, ids + ind, prim_id, isect[k].uvwt);
        float4 norm = ConvertFromBarycentric3(normals + ind*3, ids + ind, prim_id, isect[k].uvwt);
        norm = normalize(norm);

        float4 dir = light - pos;
        rays[k].d = normalize(dir);
        rays[k].o = pos + norm * EPSILON;
        rays[k].o.w = length(dir);

        rays[k].extra.x = 0xFFFFFFFF;
        rays[k].extra.y = 0xFFFFFFFF;
   }
}


__kernel void Shading(//scene
                __global float* positions,
                __global float* normals,
				__global float* texcoords,
                __global int* ids,
                __global float* ambient,
                __global float* diffuse,
				__global unsigned char* texturePool,
                __global int* indents,
				__global Texture* textures,
                //intersection
                __global Intersection* isect,
                __global const int* occl,
                //light pos
                float4 light,
                int width,
                int height,
                __global unsigned char* out)
{
    int2 globalid;
    globalid.x  = get_global_id(0);
    globalid.y  = get_global_id(1);

    // Check borders
    if (globalid.x < width && globalid.y < height)
    {
        int k = globalid.y * width + globalid.x;
        int shape_id = isect[k].shapeid;
        int prim_id = isect[k].primid;
        float4 col = {0.7, 1.0, 1.0, 1.0}; //Background color

        if (shape_id != -1 && prim_id != -1)
        {
            // Calculate position and normal of the intersection point
            int ind = indents[shape_id];

            float4 pos = ConvertFromBarycentric3(positions + ind*3, ids + ind, prim_id, isect[k].uvwt);
            float4 norm = ConvertFromBarycentric3(normals + ind*3, ids + ind, prim_id, isect[k].uvwt);
            norm = normalize(norm);

			//triangle diffuse color
			int color_id = ind + prim_id*3;
			float4 amb_col  = (float4)( ambient[color_id],
					                    ambient[color_id + 1],
						                ambient[color_id + 2], 1.f);
			float4 diff_col = (float4)( diffuse[color_id],
					                    diffuse[color_id + 1],
						                diffuse[color_id + 2], 1.f);

			//triangle texture (if any)
			int texture_id = ind + prim_id;
			unsigned int w = textures[texture_id].w;
			unsigned int h = textures[texture_id].h;

			if (w > 0 && h > 0) {

				//compute pixel texture coordinates (nearest filtering)
				float2 uv = ConvertFromBarycentric2(texcoords + ind*2, ids + ind, prim_id, isect[k].uvwt);
				uv -= floor(uv);
				unsigned int s = clamp((unsigned int)floor(uv.x * (float)w), 0u, w - 1u);
				unsigned int t = clamp((unsigned int)floor(uv.y * (float)h), 0u, h - 1u);

				//fetch textel
				__global uchar const* texdata = texturePool + textures[texture_id].offset;
				diff_col.x = (float)texdata[0 + 3 * (w * t + s)] / 255.f;
				diff_col.y = (float)texdata[1 + 3 * (w * t + s)] / 255.f;
				diff_col.z = (float)texdata[2 + 3 * (w * t + s)] / 255.f;
				amb_col.xyz = 0.2f * diff_col.xyz;
			}

			// Calculate lighting
            col = amb_col;

            if (occl[k] == -1)
            {
                float4 light_dir = normalize(light - pos);
                float dot_prod = dot(norm, light_dir);
                if (dot_prod > 0.f)
                    col += dot_prod * diff_col;
            }
        }

		// Clamp to 0.0 - 1.0
		if (col.x > 1.0f) col.x = 1.0f;
		if (col.y > 1.0f) col.y = 1.0f;
		if (col.z > 1.0f) col.z = 1.0f;

		// Output
        out[k * 4] = col.x * 255;
        out[k * 4 + 1] = col.y * 255;
        out[k * 4 + 2] = col.z * 255;
        out[k * 4 + 3] = 255;
    }
}

#endif //KERNEL_CL
