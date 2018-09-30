
#version 460

in vec3 ioColor;

void main()
{
  gl_FragColor = vec4(ioColor, 1.0);

}

