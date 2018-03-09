#version 420
 
layout (std140, binding = 1) uniform Material {
    vec4 diffuse;
    vec4 ambient;
    vec4 specular;
    //    vec4 emissive;
    vec4 auxilary;
};

/*
 * auxilary[0] is the shininess of the object
 * auxilary[1] is 0.0 if the object has no diffuse map
 * auxilary[1] is 1.0 if the object has a diffuse map
 * auxilary[1] is 2.0 if the object has a diffuse map and a normal map
 * auxilary[1] is 3.0 if the object has a diffuse map, a normal map and a specular map
 * auxilary[1] is 4.0 if the object has a diffuse map and a specular map
 * auxilary[2] is the roughness of the object
 *
 */

layout (binding = 0) uniform sampler2D texUnit;
layout (binding = 1) uniform sampler2D nrmUnit;
layout (binding = 2) uniform sampler2D spcUnit;

in VS_OUT
{
  vec3 N;
  vec3 L;
  vec3 V;
  vec2 TexCoord;
  vec3 T;
  vec3 B;
} fs_in;

layout (location = 0) out vec4 color;

void main()
{
    vec4 diffuse_color;
    vec4 ambient_color;
    vec4 specular_color;

    //Compute normal vector (from normal map, if available)
    vec3 normDir;
    if (auxilary[1] == 2.0 || auxilary[1] == 3.0) {
    	vec3 normal = 2.0 * texture(nrmUnit, fs_in.TexCoord).rgb - 1.0;
	normDir = normalize(mat3(fs_in.T, fs_in.B, fs_in.N) * normal);
    }	
    else {
        normDir = normalize(fs_in.N);
    }

    //Compute and normalize light, view and half-dir vectors
    vec3 lightDir = normalize(fs_in.L);
    vec3 viewDir  = normalize(fs_in.V);
    vec3 halfDir  = normalize(lightDir + viewDir);

    //Lighting equations
    //float LdotV = dot(lightDir, viewDir);
    float NdotL = dot(normDir, lightDir);
    //float NdotV = dot(normDir, viewDir);
    float NdotH = dot(normDir, halfDir);
 
    if (auxilary[1] == 0.0)
      {
	diffuse_color = diffuse;
	ambient_color = ambient;
      }
    if (auxilary[1] > 0.0)
      {
	diffuse_color = texture2D(texUnit, fs_in.TexCoord);
	ambient_color = 0.33 * ambient;
      }
    color.a = diffuse_color.a;
    diffuse_color *= max(0.0, NdotL);
    
    if (auxilary[1] > 2.0)
       specular_color = texture2D(spcUnit, fs_in.TexCoord);
    else
       specular_color = specular;

    specular_color *= pow(max(0.0, NdotH), auxilary[0]);
    color.rgb = (specular_color + diffuse_color  + ambient_color).rgb;
}
