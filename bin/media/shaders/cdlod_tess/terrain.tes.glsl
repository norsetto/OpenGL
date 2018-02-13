#version 420

layout (quads, fractional_even_spacing) in;

//layout (binding = 0) uniform usampler2D tex_displacement;
layout (binding = 0) uniform sampler2D tex_displacement;

uniform float dmap_depth = 0.4;

in TCS_OUT
{
  vec2 tc;
} tes_in[];

out TES_OUT
{
  vec3 pos;
  vec3 eye;
  vec3 light;
  vec2 normal;
} tes_out;

layout (std140) uniform Matrices
{
  mat4 view_matrix;
  mat4 proj_matrix;
  mat4 vp_matrix;
  vec4 light_pos;
};

void main(void)
{

  vec2 tc1 = mix(tes_in[1].tc, tes_in[3].tc, gl_TessCoord.x);
  vec2 tc2 = mix(tes_in[0].tc, tes_in[2].tc, gl_TessCoord.x);
  vec2 tc = mix(tc1, tc2, gl_TessCoord.y);

  vec4 p1 = mix(gl_in[1].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);
  vec4 p2 = mix(gl_in[0].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x);
  vec4 p = mix(p1, p2, gl_TessCoord.y);
  //  p.y = float(texture(tex_displacement, tc).r) * dmap_depth / 65535.0;
  vec4 texel = texture(tex_displacement, tc); 
  p.y = (texel.r * 256.0 + texel.g) * dmap_depth / 257.0;
  
  gl_Position = vp_matrix * p;

  tes_out.pos = p.xyz;
  tes_out.eye = -(view_matrix * p).xyz;
  tes_out.normal = 2.0 * texel.ba - 1.0;
  tes_out.light =  mat3(view_matrix) * (light_pos.xyz - p.xyz);
}
