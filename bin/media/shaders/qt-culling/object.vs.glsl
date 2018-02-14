#version 330

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoord;

uniform mat4 view_matrix;
uniform mat4 proj_matrix;
uniform mat4 model_matrix;

out VS_OUT
{
  vec2 TexCoord;
} vs_out;

void main()
{
  vs_out.TexCoord = texCoord;
  gl_Position = proj_matrix * view_matrix * model_matrix * vec4(position, 1.0);
}
    
