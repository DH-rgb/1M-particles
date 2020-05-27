#version 120
//
// simple.vert
//
attribute vec3 point, motion;
uniform mat4 transformMatrix;
uniform float elapsedTime;
uniform float mousex;
uniform float mousey;
uniform float velocity;
uniform bool mouseFlag;
varying vec3 position, vector;
const float speed = 5.0;

float rand(vec2 co){
  return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

void main(void)
{
  vec3 mouse = vec3(mousex,-mousey,0.0);
  vec3 v = normalize(mouse - point.xyz)*0.2;//カーソル位置へのベクトル
  vec3 w = normalize(v + motion.xyz);//ハーフベクトルで向きを補正
  vec3 destposition = vec3(point.xyz + w * speed * velocity);//次の位置
  vec3 destvector = w;

  gl_Position = transformMatrix * vec4(point.xyz,1.0);
  
  if(!mouseFlag){destvector.xyz = motion.xyz;}
  position = destposition;  // 点の位置を更新する
  vector = destvector;//速度の更新
}
