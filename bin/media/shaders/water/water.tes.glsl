#version 410 core

layout (quads, fractional_even_spacing) in;

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

in TCS_OUT
{
  vec2 tc;
} tes_in[];

out TES_OUT
{
  vec2 tc;
  vec3 pos;
  vec3 eye;
  vec3 light;
  vec3 normal;
} tes_out;

//
// Description : Array and textureless GLSL 2D/3D/4D simplex 
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : stegu
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//               https://github.com/stegu/webgl-noise
// 

vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
  return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
{ 
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

  // First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;

  // Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  //   x0 = x0 - 0.0 + 0.0 * C.xxx;
  //   x1 = x0 - i1  + 1.0 * C.xxx;
  //   x2 = x0 - i2  + 2.0 * C.xxx;
  //   x3 = x0 - 1.0 + 3.0 * C.xxx;
  vec3 x1 = x0 - i1 + C.xxx;
  vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
  vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

  // Permutations
  i = mod289(i); 
  vec4 p = permute( permute( permute( 
				     i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
			     + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
		    + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

  // Gradients: 7x7 points over a square, mapped onto an octahedron.
  // The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
  float n_ = 0.142857142857; // 1.0/7.0
  vec3  ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
  //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

  //Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

  // Mix final noise value
  vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
                                dot(p2,x2), dot(p3,x3) ) );
}

void main(void)
{
  vec2 tc1 = mix(tes_in[0].tc, tes_in[1].tc, gl_TessCoord.x);
  vec2 tc2 = mix(tes_in[2].tc, tes_in[3].tc, gl_TessCoord.x);
  vec2 tc  = mix(tc1, tc2, gl_TessCoord.y);
  
  vec4 p1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
  vec4 p2 = mix(gl_in[2].gl_Position, gl_in[3].gl_Position, gl_TessCoord.x);
  vec4 p  = mix(p1, p2, gl_TessCoord.y);

  vec3 wave = vec3(p.x, 0, p.z);
  vec3 normal = vec3(0.0, 1.0, 0.0);
  float sim_time = light_pos.w;

  float Phase = dot(D1.xy, p.xz) * frequency.x + phase.x * sim_time;
  float CosPhase = cos(Phase);
  float SinPhase = sin(Phase);
  wave += amplitude.x * vec3(Q.x * CosPhase * D1.x,
			     SinPhase,
			     Q.x * CosPhase * D1.y);
  normal -= amplitude.x * frequency.x * vec3(D1.x * CosPhase,
					     Q.x * SinPhase,
					     D1.y * CosPhase);

  Phase = dot(D1.zw, p.xz) * frequency.y + phase.y * sim_time;
  CosPhase = cos(Phase);
  SinPhase = sin(Phase);
  wave += amplitude.y * vec3(Q.y * CosPhase * D1.z,
			     SinPhase,
			     Q.y * CosPhase * D1.w);
  normal -= amplitude.y * frequency.y * vec3(D1.z * CosPhase,
					     Q.y * SinPhase,
					     D1.w * CosPhase);

  Phase = dot(D2.xy, p.xz) * frequency.z + phase.z * sim_time;
  CosPhase = cos(Phase);
  SinPhase = sin(Phase);
  wave += amplitude.z * vec3(Q.z * CosPhase * D2.x,
			     SinPhase,
			     Q.z * CosPhase * D2.y);
  normal -= amplitude.z * frequency.z * vec3(D2.x * CosPhase,
					     Q.z * SinPhase,
					     D2.y * CosPhase);

  Phase = dot(D2.zw, p.xz) * frequency.w + phase.w * sim_time;
  CosPhase = cos(Phase);
  SinPhase = sin(Phase);
  wave += amplitude.w * vec3(Q.w * CosPhase * D2.z,
			     SinPhase,
			     Q.w * CosPhase * D2.w);
  normal -= amplitude.w * frequency.w * vec3(D2.z * CosPhase,
					     Q.w * SinPhase,
					     D2.w * CosPhase);

  // Perturb the coords with three components of noise
  vec3 uvw = wave + 0.05 * vec3(snoise(wave + vec3(0.0, 0.0, sim_time)),
				snoise(wave + vec3(43.0, 17.0, sim_time)),
				snoise(wave + vec3(-17.0, -43.0, sim_time)));
  
  // Six components of noise in a fractal sum
  float n = snoise(uvw - vec3(0.0, 0.0, sim_time));
  n += 0.5 * snoise(uvw * 2.0 - vec3(0.0, 0.0, sim_time * 1.4)); 
  n += 0.25 * snoise(uvw * 4.0 - vec3(0.0, 0.0, sim_time * 2.0)); 
  n += 0.125 * snoise(uvw * 8.0 - vec3(0.0, 0.0, sim_time * 2.8)); 
  n += 0.0625 * snoise(uvw * 16.0 - vec3(0.0, 0.0, sim_time * 4.0)); 
  n += 0.03125 * snoise(uvw * 32.0 - vec3(0.0, 0.0, sim_time * 5.6));
  
  wave += vec3(n) * 0.075;
  normal += vec3(-n, 2.0 * n, -n) * 0.1;
  
  tes_out.tc = tc;
  tes_out.pos = wave;
  tes_out.normal = normal;
  vec4 P = view_matrix * vec4(wave, p.w);
  tes_out.eye = -P.xyz;
  tes_out.light = mat3(view_matrix) * (light_pos.xyz - wave);
  gl_Position = proj_matrix * P;
}
