
#version 460

uniform mat4 MVP;

in vec3 vNormal;
in vec3 vPos;
in vec2 vTexCoord_0;

out vec3 ioNormal;
out vec2 ioTexCoord_0;

void main()
{
  gl_Position = MVP * vec4(vPos, 1.0);
  ioNormal = vNormal;
  ioTexCoord_0 = vTexCoord_0;
};
