#version 420

layout (binding = 0) uniform sampler2D texture1;
layout (binding = 1) uniform sampler2D texture2;

layout (location = 0) out vec4 outFragColor;

in GS_OUT
{
  vec2  tc;
  float rotation;
  vec4  color;
  flat uint type;
} fs_in;

float center_color = 0.8f;
float border_color = 0.2f;

void main()
{
  // Rotate texture coordinates
  float rotCenter = 0.5;
  float rotCos = cos(fs_in.rotation);
  float rotSin = sin(fs_in.rotation);
  vec2  rotTc = vec2(rotCos * (fs_in.tc.x - rotCenter) + rotSin * (fs_in.tc.y - rotCenter) + rotCenter,
		     rotCos * (fs_in.tc.y - rotCenter) - rotSin * (fs_in.tc.x - rotCenter) + rotCenter);
  
  //Distance from center of quad
  float dist =  length(fs_in.tc - vec2(0.5));

  //Narrow the border size
  dist *= 2.0 * dist;

  //Mix between center and border color
  vec4 color = fs_in.color * mix(center_color, border_color, dist);

  //Blend with texture
  vec4 texture_color;
  if (fs_in.type == 0)
    texture_color = texture(texture1, rotTc);
  else
    texture_color = texture(texture2, rotTc);
  outFragColor = texture_color * color;
}
