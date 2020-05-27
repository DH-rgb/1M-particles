#version 120
//
// simple.frag
//
uniform float ambient;

void main(void)
{
  gl_FragColor = vec4(ambient,1.0,0.8,1.0);
}
