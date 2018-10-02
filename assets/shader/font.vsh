#version 460

in highp vec2 vPos;
in vec4 vColor;
in highp vec2 vTexCoord_0;

out vec4 ioColor;
out highp vec2 ioTexCoord_0;

void main()
{
  ioColor = vColor;
  ioTexCoord_0 = vTexCoord_0;
  gl_Position = vec4(vPos.x, vPos.y, 0, 1);
};
