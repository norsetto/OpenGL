#version 330
#extension GL_ARB_shading_language_420pack: enable

layout (std140, binding = 1) uniform Material {
    vec4 diffuse;
    vec4 ambient;
    vec4 specular;
    //    vec4 emissive;
    vec4 auxilary;
};

/*
 * auxilary[0] is the shininess of the object
 * auxilary[1] is not zero if the object has a texture map
 * auxilary[2] is the roughness of the object
 *
 */

uniform sampler2D texUnit;

in VS_OUT
{
  vec2 TexCoord;
} fs_in;

layout (location = 0) out vec4 color;

void main()
{
    if (auxilary[1] == 0.0)
      {
	color = ambient;
      }
    else
      {
	color = texture2D(texUnit, fs_in.TexCoord);
      }
}
