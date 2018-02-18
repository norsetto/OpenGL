#version 420

layout (location = 0) in vec3 position;

uniform float water_size = 4.0;
uniform int patch_size = 64;

out VS_OUT {
    vec2 tc;
} vs_out;

void main() {
  
  const vec3 vertices[] = vec3[](vec3(0.0, 0.0, 0.0),
				 vec3(0.0, 0.0, 1.0),
				 vec3(1.0, 0.0, 0.0),
				 vec3(1.0, 0.0, 1.0));

  //Position patch in 2D grid based on instance ID
  int ix = gl_InstanceID / patch_size;
  int iz = gl_InstanceID % patch_size;

  vec3 offset = vec3(float(ix), 0.0, float(iz));

  vs_out.tc = ( vertices[gl_VertexID].xz + offset.xz ) / float(patch_size);
  gl_Position = vec4( (( vertices[gl_VertexID] + offset ) / float(patch_size) - 0.5) * water_size , 1.0);
}
