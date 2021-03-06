#version 420

layout (vertices = 4) out;

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

in VS_OUT
{
    vec2 tc;
} tcs_in[];

out TCS_OUT
{
    vec2 tc;
} tcs_out[];

uniform int grid = 3;
uniform float max_distance = 15.0;
uniform vec2 viewport = vec2(1920.0, 1080.0);

#if 1
  //Camera Distance Approach
  float GetTessLevel(float Distance0, float Distance1) {
  float AvgDistance = (Distance0 + Distance1) * 0.5;
  float TessLevel = 1.0;
  float dist = max_distance;
  float amplitude = 1.0;
  
  for (int i = 0; i < grid; i++) {
    TessLevel += amplitude * step(AvgDistance, dist);
    dist /= 2.0;
    amplitude *= 2.0;
  }
  return TessLevel;
}
#else
  //Screen Space Distance Approach
  float GetTessLevel(vec4 p0, vec4 p1) {

  //Project to screen space
  vec4 view0 = view_matrix * p0;
  vec4 view1 = view0;
  view1.x += distance(p0, p1);
	
  //Then clip space and screen space
  vec4 clip0 = proj_matrix * view0;
  vec4 clip1 = proj_matrix * view1;

  clip0 /= clip0.w;
  clip1 /= clip1.w;

  vec2 screen0 = ((clip0.xy + 1.0) * 0.5) * viewport;
  vec2 screen1 = ((clip1.xy + 1.0) * 0.5) * viewport;

  //Length of edge in screen space
  float d = distance(screen0, screen1);
  
  //Compute tesselation factor vs. the desidered target (2^grid)
  return d / exp2(float(grid));
}
#endif

void main(void)
{
  if (gl_InvocationID == 0) {
#if 1
    //Compute displaced control points
    float EyeToVertexDistance[4];
    for (uint i = 0; i < 4; i++) {
      //Calculate the distance from the camera to the four control points
      EyeToVertexDistance[i] = length((view_matrix * gl_in[i].gl_Position).xyz);
    }
    // Calculate the outer tessellation levels
    gl_TessLevelOuter[0] = GetTessLevel(EyeToVertexDistance[0], EyeToVertexDistance[1]);
    gl_TessLevelOuter[1] = GetTessLevel(EyeToVertexDistance[1], EyeToVertexDistance[3]);
    gl_TessLevelOuter[2] = GetTessLevel(EyeToVertexDistance[2], EyeToVertexDistance[3]);
    gl_TessLevelOuter[3] = GetTessLevel(EyeToVertexDistance[0], EyeToVertexDistance[2]);
#else

    //Calculate the outer tessellation levels
    gl_TessLevelOuter[0] = GetTessLevel(gl_in[0].gl_Position, gl_in[1].gl_Position);
    gl_TessLevelOuter[1] = GetTessLevel(gl_in[1].gl_Position, gl_in[3].gl_Position);
    gl_TessLevelOuter[2] = GetTessLevel(gl_in[2].gl_Position, gl_in[3].gl_Position);
    gl_TessLevelOuter[3] = GetTessLevel(gl_in[0].gl_Position, gl_in[2].gl_Position);
#endif
  
  //Calculate the inner tessellation levels
  gl_TessLevelInner[0] = (gl_TessLevelOuter[0] + gl_TessLevelOuter[2]) * 0.5;
  gl_TessLevelInner[1] = (gl_TessLevelOuter[1] + gl_TessLevelOuter[3]) * 0.5;
  }
  //Output
  gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
  tcs_out[gl_InvocationID].tc = tcs_in[gl_InvocationID].tc;
}
