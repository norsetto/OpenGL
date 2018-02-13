#version 420
 
layout (location = 0) out vec4 color;

uniform float dmap_depth = 0.4;
uniform float nmap_depth = 0.05;

const vec3 brown = vec3(0.75, 0.6, 0.5);
const vec3 green = vec3(0.0, 0.65, 0.3);

in TES_OUT
{
  vec3 pos;
  vec3 eye;
  vec3 light;
  vec2 normal;
} fs_in;

void main()
{
  vec3 L = normalize(fs_in.light);
  vec3 N = normalize(vec3(fs_in.normal.x, nmap_depth, fs_in.normal.y));
  float NdotL = max(0.0, dot(N, L));

  color = vec4(NdotL * mix(brown, green, N.y), 1.0);

  float z = length(fs_in.eye);

  float de = 0.2 * smoothstep(0.0, dmap_depth, dmap_depth - fs_in.pos.y);
  float di = 0.1 * smoothstep(0.0, dmap_depth, dmap_depth - fs_in.pos.y);
		
  float extintion = exp(-z * de);
  float inscattering = exp(-z * di);
  const vec4 FOG_COLOR = vec4(0.7, 0.8, 0.9, 1.0);
  color = color * extintion + FOG_COLOR * (1.0 - inscattering);
}
