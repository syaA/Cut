
#version 460

in highp vec2 ioTexCoord_0;

out vec4 FragColor;

uniform sampler2D gui_sampler;

uniform vec4 color0;
uniform vec4 color1;


void main()
{
  vec4 tex = texture(gui_sampler, ioTexCoord_0);
  vec4 col = mix(color0, color1, tex.rgbr);
  col.a *= tex.a;

  FragColor = col;
}

