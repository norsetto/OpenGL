#version 420

layout (points) in;
layout (triangle_strip) out;
layout (max_vertices = 8) out;

layout (std140, binding = 0) uniform Matrices
{
    mat4 view_matrix;
    mat4 proj_matrix;
    mat4 vp_matrix;
    vec4 light_pos;
};

in VS_OUT
{
  vec3 properties;
} gs_in[];

out GS_OUT
{
  vec2 tex_coord;
} gs_out;

float Triangle(float x)
{
  return abs(fract(x) * 2.0 - 1.0);
}

void main()
{
  //Input point
  vec3 P = gl_in[0].gl_Position.xyz;

  //Compute the angle of motion
  //cos(x) = 1.0 - 2.0 * smoothstep(0.0, 1.0, Triangle(x/(2.0*pi()) + 0.50));
  //sin(x) = 1.0 - 2.0 * smoothstep(0.0, 1.0, Triangle(x/(2.0*pi()) + 0.25));
  //float phi = 1.57 * Triangle(0.5 * light_pos.w);
  //float cos_phi = cos(phi);
  //float sin_phi = sin(phi);
  float t = gs_in[0].properties.x * light_pos.w + gs_in[0].properties.y;
  float cos_phi = 1.0 - 2.0 * smoothstep(0.0, 1.0, Triangle(t + 0.50));
  float sin_phi = 1.0 - 2.0 * smoothstep(0.0, 1.0, Triangle(t + 0.25));
  /*
  mat3 rot_phi_plus = mat3(cos_phi, -sin_phi, 0.0,
		           sin_phi,  cos_phi, 0.0,
		               0.0,      0.0, 1.0);
  mat3 rot_phi_min  = mat3( cos_phi,  sin_phi, 0.0,
		           -sin_phi,  cos_phi, 0.0,
		               0.0,       0.0, 1.0);
  */
  //Left wing
  //Emit new vertex at relative (0,0) position
  //vec3 v = P - rot_phi_plus * vec3(0.5, 0.0, 0.5);
  vec3 v = P - 0.5 * vec3(cos_phi, -sin_phi, 1.0);
  gl_Position = vp_matrix * vec4(v, gl_in[0].gl_Position.w);
  gs_out.tex_coord = vec2(0.0, 0.0);
  EmitVertex();

  //Emit new vertex at relative (1,0) position
  v = P - vec3(0.0, 0.0, 0.5);
  gl_Position = vp_matrix * vec4(v, gl_in[0].gl_Position.w);
  gs_out.tex_coord = vec2(0.5, 0.0);
  EmitVertex();

  //Emit new vertex at relative (0,1) position
  //v = P + rot_phi_plus * vec3(-0.5, 0.0, 0.5);
  v = P + 0.5 * vec3(-cos_phi, sin_phi, 1.0);
  gl_Position = vp_matrix * vec4(v, gl_in[0].gl_Position.w);
  gs_out.tex_coord = vec2(0.0, 1.0);
  EmitVertex();

  //Emit new vertex at relative (1,1) position
  v = P + vec3(0.0, 0.0, 0.5);
  gl_Position = vp_matrix * vec4(v, gl_in[0].gl_Position.w);
  gs_out.tex_coord = vec2(0.5, 1.0);
  EmitVertex();

  //Right wing
  //Emit new vertex at relative (0,0) position
  v = P - vec3(0.0, 0.0, 0.5);
  gl_Position = vp_matrix * vec4(v, gl_in[0].gl_Position.w);
  gs_out.tex_coord = vec2(0.5, 0.0);
  EmitVertex();

  //Emit new vertex at relative (1,0) position
  //v = P + rot_phi_min * vec3(0.5, 0.0, -0.5);
  v = P + 0.5 * vec3(cos_phi, sin_phi, -1.0);
  gl_Position = vp_matrix * vec4(v, gl_in[0].gl_Position.w);
  gs_out.tex_coord = vec2(1.0, 0.0);
  EmitVertex();

  //Emit new vertex at relative (0,1) position
  v = P + vec3(0.0, 0.0, 0.5);
  gl_Position = vp_matrix * vec4(v, gl_in[0].gl_Position.w);
  gs_out.tex_coord = vec2(0.5, 1.0);
  EmitVertex();

  //Emit new vertex at relative (1,1) position
  //v = P + rot_phi_min * vec3(0.5, 0.0, 0.5);
  v = P + 0.5 * vec3(cos_phi, sin_phi, 1.0);
  gl_Position = vp_matrix * vec4(v, gl_in[0].gl_Position.w);
  gs_out.tex_coord = vec2(1.0, 1.0);
  EmitVertex();

  EndPrimitive();
}
