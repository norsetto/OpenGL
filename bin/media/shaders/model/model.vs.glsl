#version 420

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

layout (std140, binding = 0) uniform Matrices
{
    mat4 model_matrix;
    mat4 view_matrix;
    mat4 proj_matrix;
    vec3 light_pos;
};

out VS_OUT
{
  vec3 N;
  vec3 L;
  vec3 V;
  vec2 TexCoord;
  vec3 T;
  vec3 B;
} vs_out;

void main()
{
  mat4 modelview_matrix = view_matrix * model_matrix;
  vec4 P = modelview_matrix * vec4(position, 1.0);
  vs_out.N = mat3(modelview_matrix) * normal;
  vs_out.L = light_pos - P.xyz;
  vs_out.V = -P.xyz;
  vs_out.TexCoord = texCoord;
  vs_out.T = mat3(modelview_matrix) * tangent;
  vs_out.B = mat3(modelview_matrix) * bitangent;
  gl_Position = proj_matrix * P;
}
