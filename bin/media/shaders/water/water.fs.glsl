#version 410 core

out vec4 color;

uniform sampler2D tex_color;

layout (std140) uniform Matrices
{
  mat4 view_matrix;
  mat4 proj_matrix;
  mat4 vp_matrix;
  vec4 light_pos;
  vec4 amplitude;
  vec4 frequency;
  vec4 phase;
  vec4 Q;
  vec4 D1;
  vec4 D2;
};

in TES_OUT
{
  vec2 tc;
  vec3 pos;
  vec3 eye;
  vec3 light;
  vec3 normal;
} fs_in;

uniform vec4 water_color = vec4(0.00, 0.412, 0.58, 1.0);
uniform float ambient_albedo = 0.1;
uniform float diffuse_albedo = 0.4;
uniform float specular_albedo = 0.4;
uniform float specular_power = 10.0;

void main(void)
{
  /*
  vec2 translate = vec2(mod(sim_time, 370.0)*0.0025, 0.0);
  vec4 bump1 = texture(bumpmap, fs_in.tc * 1.00 + translate * 2.0);
  vec4 bump2 = texture(bumpmap, fs_in.tc * 2.00 + translate * 4.0);
  vec4 bump3 = texture(bumpmap, fs_in.tc * 4.00 + translate * 2.0);
  vec4 bump4 = texture(bumpmap, fs_in.tc * 8.00 + translate);
  vec4 bump = normalize(2.0 * (bump1 + bump2 + bump3 + bump4) - vec4(4.0));
  
  vec3 normDir  = normalize(fs_in.normal) + vec3(bump.xy, 0);
  */
  vec3 normDir  = normalize(fs_in.normal);
  vec3 lightDir = normalize(fs_in.light);
  vec3 viewDir  = normalize(fs_in.eye);

  // diffuse component
  float NdotL   = dot(normDir, lightDir);
  float diffuse_color = max(NdotL, 0.0) * diffuse_albedo;

  // specular component
  vec3 reflDir = reflect(lightDir, normDir);
  float specular_color = pow(max(dot(reflDir, viewDir), 0.0), specular_power) * specular_albedo;

  //color = texture(tex_color, fs_in.tc) * (diffuse_color + specular_color + ambient_albedo);
  color = water_color * (diffuse_color + specular_color + ambient_albedo);
  //color = vec4(normDir, 1.0);
}
