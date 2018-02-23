#version 420

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in VS_OUT
{
  float rotation;
  vec4  color;
  float size;
  uint  type;
} gs_in[];

out GS_OUT
{
  vec2 tc;
  float rotation;
  vec4  color;
  flat uint  type;
} gs_out;

layout (std140, binding = 0) uniform ubo
{
    mat4 view_matrix;
    mat4 proj_matrix;
    mat4 vp_matrix;
};

void main()
{
  //Input point
  vec4 P = view_matrix * gl_in[0].gl_Position;

  gl_Position = proj_matrix * (P - vec4( 1.0,  1.0, 0.0, 0.0) * gs_in[0].size);
  gs_out.tc = vec2(0.0, 0.0);
  gs_out.rotation = gs_in[0].rotation;
  gs_out.color = gs_in[0].color;
  gs_out.type  = gs_in[0].type;
  EmitVertex();

  gl_Position = proj_matrix * (P + vec4(-1.0,  1.0, 0.0, 0.0) * gs_in[0].size);
  gs_out.tc = vec2(0.0, 1.0);
  gs_out.rotation = gs_in[0].rotation;
  gs_out.color = gs_in[0].color;
  gs_out.type  = gs_in[0].type;
  EmitVertex();

  gl_Position = proj_matrix * (P + vec4( 1.0, -1.0, 0.0, 0.0) * gs_in[0].size);
  gs_out.tc = vec2(1.0, 0.0);
  gs_out.rotation = gs_in[0].rotation;
  gs_out.color = gs_in[0].color;
  gs_out.type  = gs_in[0].type;
  EmitVertex();

  gl_Position = proj_matrix * (P + vec4( 1.0,  1.0, 0.0, 0.0) * gs_in[0].size);
  gs_out.tc = vec2(1.0, 1.0);
  gs_out.rotation = gs_in[0].rotation;
  gs_out.color = gs_in[0].color;
  gs_out.type  = gs_in[0].type;
  EmitVertex();

  EndPrimitive();
}
