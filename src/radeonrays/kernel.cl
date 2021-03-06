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

float3 lerp(float3 a, float3 b, float w)
{
	return a + w*(b - a);
}

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
					//colors
					__global float3* color,
					__global float3* light_col,
					__global float3* blend_col,
					const float4 cam_pos,
                    const float4 cam_forward,
                    const float4 cam_right,
                    const float4 cam_up,
                    const float4 cam_zcap,
                    int width,
                    int height,
					__global int* numrays)
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

		color[k].x = 0.0f;
		color[k].y = 0.0f;
		color[k].z = 0.0f;

		blend_col[k].x = 1.0f;
		blend_col[k].y = 1.0f;
		blend_col[k].z = 1.0f;

		light_col[k].x = 1.0f;
		light_col[k].y = 1.0f;
		light_col[k].z = 1.0f;

		atomic_inc(numrays);
    }
}

__kernel void GenerateSecondaryRays(__global Ray* rays,
				//scene
				__global float* normals,
				__global int* ids,
                __global int* indents,
                __global float* ior,
				__global float* diffuse,
				__global float* specular,
                //intersection
                __global Intersection* isect,
                int width,
                int height,
				__global int* numrays)
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

        if (shape_id != -1 && prim_id != -1)
        {
            int ind = indents[shape_id];
			int color_id = ind + prim_id*3;

			// compute ratio of index of reflections (1.0f is vacuum~air)
			float eta = 1.0f / ior[ind / 3 + prim_id];

			if (eta != 1.0f) {

				// Increment number of rays
				atomic_inc(numrays);

				// compute normal at intersection point
			    float4 norm = ConvertFromBarycentric3(normals + ind*3, ids + ind, prim_id, isect[k].uvwt);
				norm = normalize(norm);

				// compute cosine of angle between incident ray and local normal
				float ndoti = -dot(norm.xyz, rays[k].d.xyz);

				// revert the ratio of refraction indices if we are exiting the surface
				if (ndoti < 0.f) {
					eta = 1.f / eta;
				}

				float kappa = 1.0f - eta * eta * (1.0f - ndoti * ndoti);

				if (kappa >= 0.f) {

					float3 refract = eta * rays[k].d.xyz - (eta * ndoti + sqrt(kappa)) * norm.xyz;

					//Set new ray origin
					rays[k].o.xyz += (isect[k].uvwt.w + EPSILON) * rays[k].d.xyz;

					//Set new ray direction
					rays[k].d.xyz = normalize(refract);

				} else {
					//Set new ray origin
					rays[k].o.xyz += (isect[k].uvwt.w + EPSILON) * rays[k].d.xyz;
				}

			} else if (specular[color_id + 0] > 0.0f ||
					   specular[color_id + 1] > 0.0f ||
					   specular[color_id + 2] > 0.0f)
			{
				// compute normal at intersection point
			    float4 norm = ConvertFromBarycentric3(normals + ind*3, ids + ind, prim_id, isect[k].uvwt);
				norm = normalize(norm);

				// compute cosine of angle between incident ray and local normal
				float ndoti = dot(norm.xyz, rays[k].d.xyz);

				// compute reflected ray
				float3 reflect = rays[k].d.xyz - 2.0f * ndoti * norm.xyz;

				//Set new ray origin
				rays[k].o.xyz += isect[k].uvwt.w * rays[k].d.xyz;

				//Set new ray direction
				rays[k].d.xyz = normalize(reflect);

				//Offset new ray origin
				rays[k].o.xyz += EPSILON * rays[k].d.xyz;

				// Increment number of rays
				atomic_inc(numrays);

			} else {
				//Set ray to inactive
				rays[k].extra.y = 0x00000000;
			}
		}
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

__kernel void GenerateSecondaryShadowRays(__global Ray* rays,
				__global float3* light_col,
				//scene
                __global int* indents,
                __global float* ior,
				__global float* diffuse,
                //intersection
                __global Intersection* isect,
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

        if (shape_id != -1 && prim_id != -1)
        {
            int ind = indents[shape_id];

			if (ior[ind / 3 + prim_id] != 1.0f) {

				//Set new ray origin and length
				rays[k].o.xyz += (isect[k].uvwt.w + EPSILON) * rays[k].d.xyz;
				rays[k].o.w -= isect[k].uvwt.w;

				//Blend light color
				int color_id = ind + prim_id * 3;
				light_col[k].x *= diffuse[color_id + 0];
				light_col[k].y *= diffuse[color_id + 1];
				light_col[k].z *= diffuse[color_id + 2];

			} else {

				//Set ray to inactive
				rays[k].extra.y = 0x00000000;
			}
		}
    }
}

__kernel void Shading(__global Ray* rays,
					  __global Ray* shadow_rays,
	//colors
	__global float3* light_col,
	__global float3* blend_col,
	//scene
	__global float* positions,
	__global float* normals,
	__global float* texcoords,
	__global int* ids,
	__global float* ambient,
	__global float* diffuse,
	__global float* specular,
	__global float* ior,
	__global unsigned char* texturePool,
	__global int* indents,
	__global Texture* textures,
	//intersection
	__global Intersection* isect,
	__global const Intersection* occl,
	//light pos
	float4 light,
	int width,
	int height,
	__global float3* color)
{
	int2 globalid;
	globalid.x = get_global_id(0);
	globalid.y = get_global_id(1);

	// Check borders
	if (globalid.x < width && globalid.y < height)
	{
		int k = globalid.y * width + globalid.x;

		if (rays[k].extra.y != 0x00000000) {

			int shape_id = isect[k].shapeid;
			int prim_id = isect[k].primid;
			float3 col = { 0.7, 1.0, 1.0 }; //Background color
			float3 diff_col = (float3)(1.0f, 1.0f, 1.0f); //Diffuse color
			float3 spec_col = (float3)(1.0f, 1.0f, 1.0f); //Specular color

			if (shape_id != -1 && prim_id != -1)
			{
				// Calculate position and normal of the intersection point
				int ind = indents[shape_id];

				float4 pos = ConvertFromBarycentric3(positions + ind * 3, ids + ind, prim_id, isect[k].uvwt);
				float4 norm = ConvertFromBarycentric3(normals + ind * 3, ids + ind, prim_id, isect[k].uvwt);
				norm = normalize(norm);

				//triangle colors
				int color_id = ind + prim_id * 3;
				float3 amb_col = (float3)(ambient[color_id + 0],
										  ambient[color_id + 1],
										  ambient[color_id + 2]);
				diff_col = (float3)(diffuse[color_id + 0],
									diffuse[color_id + 1],
									diffuse[color_id + 2]);
				spec_col = (float3)(specular[color_id + 0],
									specular[color_id + 1],
									specular[color_id + 2]);

				//triangle texture (if any)
				int texture_id = ind / 3 + prim_id;
				unsigned int w = textures[texture_id].w;
				unsigned int h = textures[texture_id].h;

				if (w > 0 && h > 0) {

					//compute pixel texture coordinates (nearest filtering)
					float2 uv = ConvertFromBarycentric2(texcoords + ind * 2, ids + ind, prim_id, isect[k].uvwt);
					uv -= floor(uv);
					unsigned int s0 = clamp((unsigned int)floor(uv.x * (float)w), 0u, w - 1u);
					unsigned int t0 = clamp((unsigned int)floor(uv.y * (float)h), 0u, h - 1u);

					// Calculate additional samples for linear filtering
					unsigned int s1 = min(s0 + 1u, w - 1u);
					unsigned int t1 = min(t0 + 1u, h - 1u);

					// Calculate weights for linear filtering
					float wx = uv.x * (float)w - floor(uv.x * (float)w);
					float wy = uv.y * (float)h - floor(uv.y * (float)h);

					//fetch texels
					__global uchar const* texdata = texturePool + textures[texture_id].offset;

					float3 sample1;
					sample1.x = (float)texdata[0 + 3 * (w * t0 + s0)] / 255.f;
					sample1.y = (float)texdata[1 + 3 * (w * t0 + s0)] / 255.f;
					sample1.z = (float)texdata[2 + 3 * (w * t0 + s0)] / 255.f;

					float3 sample2;
					sample2.x = (float)texdata[0 + 3 * (w * t0 + s1)] / 255.f;
					sample2.y = (float)texdata[1 + 3 * (w * t0 + s1)] / 255.f;
					sample2.z = (float)texdata[2 + 3 * (w * t0 + s1)] / 255.f;

					float3 sample3;
					sample3.x = (float)texdata[0 + 3 * (w * t1 + s0)] / 255.f;
					sample3.y = (float)texdata[1 + 3 * (w * t1 + s0)] / 255.f;
					sample3.z = (float)texdata[2 + 3 * (w * t1 + s0)] / 255.f;

					float3 sample4;
					sample4.x = (float)texdata[0 + 3 * (w * t1 + s1)] / 255.f;
					sample4.y = (float)texdata[1 + 3 * (w * t1 + s1)] / 255.f;
					sample4.z = (float)texdata[2 + 3 * (w * t1 + s1)] / 255.f;

					diff_col = lerp(lerp(sample1, sample2, wx), lerp(sample3, sample4, wx), wy);
					amb_col = 0.2f * diff_col;
				}

				// Calculate lighting
				col = amb_col;

				if (occl[k].shapeid == -1 && occl[k].primid == -1)
				{
					float4 light_dir = normalize(light - pos);
					float dot_prod = dot(norm, light_dir);
					if (dot_prod > 0.f) {
						col += dot_prod * diff_col * light_col[k];
					}
				}
			}
			else {
				//Set ray to inactive
				rays[k].extra.y = 0x00000000;
			}

			// Add color
			col *= blend_col[k];
			color[k] += col;

			// Blend color
			if (shape_id != -1 && prim_id != -1)
			{
				int ind = indents[shape_id];
				if (ior[ind / 3 + prim_id] != 1.0f)
					blend_col[k] *= diff_col;
				if (spec_col.x > 0.0f)
					blend_col[k].x *= spec_col.x;
				if (spec_col.y > 0.0f)
					blend_col[k].y *= spec_col.y;
				if (spec_col.z > 0.0f)
					blend_col[k].z *= spec_col.z;
			}
		}
	}
}

__kernel void ToneMapping(__global float3* color,
				float exposure,
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

		// Tone Mapping
		color[k] = (float3)1.0f - exp(-color[k] * exposure);

		// Output
        out[k * 4] = color[k].x * 255;
        out[k * 4 + 1] = color[k].y * 255;
        out[k * 4 + 2] = color[k].z * 255;
        out[k * 4 + 3] = 255;
	}
}

#endif //KERNEL_CL
