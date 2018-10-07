#version 460

in highp vec2 vPos;
in highp vec2 vTexCoord_0;

out highp vec2 ioTexCoord_0;

uniform vec2 screen_size;

void main()
{
  ioTexCoord_0 = vTexCoord_0;
  vec2 pos = vPos;
  pos.x = pos.x / screen_size.x * 2.0 - 1.0;
  pos.y = pos.y / screen_size.y * -2.0 + 1.0;
  gl_Position = vec4(pos, 0, 1);
};
