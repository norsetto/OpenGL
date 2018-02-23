#version 330

layout (location = 0) in vec3  position;
layout (location = 1) in float rotation;
layout (location = 2) in vec4  color;
layout (location = 3) in float size;
layout (location = 4) in uint  type;

out VS_OUT
{
  float rotation;
  vec4  color;
  float size;
  uint type;
} vs_out;

void main()
{
  vs_out.rotation = rotation;
  vs_out.color = color;
  vs_out.size = size;
  vs_out.type = type;
  gl_Position = vec4(position, 1.0);
}
