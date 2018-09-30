
#version 460

uniform mat4 MVP;

in vec3 vPos;
in vec3 vColor;

out vec3 ioColor;

void main()
{
  gl_Position = MVP * vec4(vPos, 1.0);
  ioColor = vColor;
};
