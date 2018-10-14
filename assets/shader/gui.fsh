
#version 460

in highp vec2 ioTexCoord_0;

out vec4 FragColor;

uniform vec4 color0;
uniform vec4 color1;
uniform float tickness;

void main()
{
  vec2 c = vec2(1.0, 1.0) - ioTexCoord_0 * 2.f;
  float v = c.x *c.x + c.y*c.y;
  float f = tickness;
  float inside = 1.0-smoothstep(1.0-f, 1.0, v) / (1.0 - f);
  float border = smoothstep(1.0-2.0*f, 1.0-f, v) * inside;

  vec3 color = mix(color1.rgb, color0.rgb, border);
  float alpha = clamp(color1.a * inside + color0.a * border, 0.0, 1.0);

  FragColor = vec4(color, alpha);
}

