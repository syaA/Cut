
#version 460

in vec4 ioColor;
in highp vec2 ioTexCoord_0;

out vec4 FragColor;

uniform sampler2D glyph_sampler;


void main()
{
  vec4 col = ioColor;
  col.a *= texture(glyph_sampler, ioTexCoord_0).r;
//  col.rgb = texture(glyph_sampler, ioTexCoord_0/10.0).rgb;

  FragColor = col;
}

