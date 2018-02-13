#version 330

uniform sampler2D color_texture;

in GS_OUT
{
  vec2 tex_coord;
} fs_in;

void main()
{
  vec4 color = texture(color_texture, fs_in.tex_coord);
  if (color.a < 0.05)
    discard;
  gl_FragColor = color;
}
