
#version 460

uniform vec4 diffuse;
uniform vec4 specular;
uniform vec4 ambient;

uniform sampler2D color_sampler;
uniform sampler2D sphere_sampler;
uniform sampler2D toon_sampler;

in vec3 ioNormal;
in vec2 ioTexCoord_0;
out vec4 FragColor;

void main()
{
  vec4 col;
  float val = clamp(dot(ioNormal, vec3(0.707, 0.707, 0.0)), 0.0, 1.0);
//  col = texture(toon_sampler, vec2(0.5, val));
  col = vec4(val, val, val, 1.f);
  col *= diffuse;
  col.rgb += ambient.rgb;
  col *= texture(color_sampler, ioTexCoord_0);

  FragColor = col;
}

