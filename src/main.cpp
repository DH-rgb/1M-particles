#include <cstdio>
#include <cstdlib>

#  define GL_GLEXT_PROTOTYPES
#  include <GL/glut.h>

/*
** 点の数
*/
#define POINTS 1000000

//ウィンドウサイズ
float WINDOW_WIDTH,WINDOW_HEIGHT;

//マウス位置
GLfloat mousex=0,mousey=0;

//転送用変数
static GLuint ambLocation;
static GLuint mouseLocationx,mouseLocationy,mouseFlagLocation;
static GLuint vLocation;
static GLuint motionLocation;
int rcount=0;
float velocity=1.0;
static int mouseFlag=0;

/*
** 実行時間
*/
static int count=1;
static GLint elapsedTimeLocation;
#define CYCLE 5000.0f

/*
**トラックボール処理
*/
#include "Trackball.h"
static Trackball trackball;
static int pressedButton;

/*
**変換行列
*/
#include "Matrix.h"
static Matrix projectionMatrix;
static Matrix modelviewMatrix;
static GLint transformMatrixLocation;

/*
** シェーダー
*/
#include "shadersource.h"
static const char vertSource[] = "simple.vert";
static const char fragSource[] = "simple.frag";
static GLuint gl2Program;

/*
** vbo確保
*/
static GLuint buffer[4];
static GLsizei points;
static GLint pointLocation;

/*
** 画面表示
*/
static void display(void)
{
  static int frame = 0;

  //投影変換　視野変換　モデリング変換
  Matrix transformMatrix = projectionMatrix * modelviewMatrix * trackball.get();
  transformMatrix.scale(1.0f, 1.0f, 1.0f);
  transformMatrix.translate(0.0f, 0.0f, -0.5f);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  //シェーダプログラム開始
  glUseProgram(gl2Program);

  //経過時間
  float elapsedTime = (float)glutGet(GLUT_ELAPSED_TIME) / CYCLE;

  //シェーダに転送
  glUniform1f(elapsedTimeLocation, elapsedTime);
  glUniformMatrix4fv(transformMatrixLocation, 1, GL_FALSE, transformMatrix.get());
  glUniform1f(ambLocation,rcount%360/360.0);

  glUniform1f(mouseLocationx,mousex);
  glUniform1f(mouseLocationy,mousey);

  glUniform1f(vLocation,velocity);

  glUniform1i(mouseFlagLocation,mouseFlag);

  glEnableVertexAttribArray(pointLocation);

  glEnableVertexAttribArray(motionLocation);

  //位置情報用のvboを指定
  glBindBuffer(GL_ARRAY_BUFFER, buffer[frame]);

  glVertexAttribPointer(pointLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

  glEnable(GL_DEPTH_TEST);

  //保存先指定
  glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, buffer[1 - frame]);

  //速度情報用のvboを指定
  glBindBuffer(GL_ARRAY_BUFFER, buffer[2 + frame]);
  
  glVertexAttribPointer(motionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);

  //保存先指定
  glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, buffer[3 - frame]);
  
  //Transform Feedback開始
  glBeginTransformFeedback(GL_POINTS);

  glDrawArrays(GL_POINTS, 0, POINTS);

  glEndTransformFeedback();

  glDisable(GL_DEPTH_TEST);

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glDisableVertexAttribArray(pointLocation);

  glUseProgram(0);

  //速度係数の更新
  if(mouseFlag){
    velocity = 1.0;
  }
  else{
    velocity*=0.95;
  }

  //buffer入れ替え
  frame = 1 - frame;

  //色更新
  rcount++;

  glutSwapBuffers();
}

/*
** ウィンドウサイズ
*/
static void resize(int w, int h)
{
  glViewport(0, 0, w, h);

  WINDOW_HEIGHT = h;
  WINDOW_WIDTH = w;

  //透視投影変換行列を求める
  projectionMatrix.loadIdentity();
  projectionMatrix.camera(80.0f, (GLfloat)w / (GLfloat)h, 5.0f, 1000.0f);
  
  trackball.region(w, h);
}


static void keyboard(unsigned char key, int x, int y)
{
  switch (key) {
    case '\033':
    case 'q':
    case 'Q':
      glDeleteBuffers(4,buffer);
      exit(0);
    default:
      break;
  }
}

static void idle(void)
{
  glutPostRedisplay();
}


static void mouse(int button, int state, int x, int y)
{
  pressedButton = button;
  
  switch (pressedButton) {
    case GLUT_RIGHT_BUTTON:
      if (state == GLUT_DOWN) {
        trackball.start(x,y);
      }
      else {
        trackball.stop(x, y);
      }
      break;
    case GLUT_LEFT_BUTTON:
      if (state == GLUT_DOWN) {
        //ドラッグ処理に渡す
        mouseFlag=1;
      }
      else {
        mouseFlag=0;
      }
      break;

    default:
      break;
  }
}


static void motion(int x, int y)
{
  switch (pressedButton) {
    case GLUT_RIGHT_BUTTON:
      trackball.motion(x, y);
      break;
    case GLUT_LEFT_BUTTON:
      //マウス位置の更新
      mousex = (x-(WINDOW_WIDTH/2.0))/WINDOW_WIDTH*100.0;
      mousey = (y-(WINDOW_HEIGHT/2.0))/WINDOW_HEIGHT*100.0;
       break;
    default:
      break;
  }
}


static void init(void)
{
  GLint compiled, linked;
  
  GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
  GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
  
  //シェーダ読み込み
  if (readShaderSource(vertShader, vertSource)) exit(1);
  if (readShaderSource(fragShader, fragSource)) exit(1);
  
  //コンパイル
  glCompileShader(vertShader);
  glGetShaderiv(vertShader, GL_COMPILE_STATUS, &compiled);
  printShaderInfoLog(vertShader);
  if (compiled == GL_FALSE) {
    fprintf(stderr, "Compile error in vertex shader.\n");
    exit(1);
  }
  
  glCompileShader(fragShader);
  glGetShaderiv(fragShader, GL_COMPILE_STATUS, &compiled);
  printShaderInfoLog(fragShader);
  if (compiled == GL_FALSE) {
    fprintf(stderr, "Compile error in fragment shader.\n");
    exit(1);
  }
  
  gl2Program = glCreateProgram();
  
  //シェーダオブジェクをプログラムに登録
  glAttachShader(gl2Program, vertShader);
  glAttachShader(gl2Program, fragShader);
  
  glDeleteShader(vertShader);
  glDeleteShader(fragShader);

  // feedback用のvarying登録
  const static char *varyings[] = { "position","vector" };
  glTransformFeedbackVaryings(gl2Program, sizeof varyings / sizeof varyings[0], varyings, GL_SEPARATE_ATTRIBS);  
  
  //リンク
  glLinkProgram(gl2Program);
  glGetProgramiv(gl2Program, GL_LINK_STATUS, &linked);
  printProgramInfoLog(gl2Program);
  if (linked == GL_FALSE) {
    fprintf(stderr, "Link error.\n");
    exit(1);
  }

  elapsedTimeLocation = glGetUniformLocation(gl2Program, "elapsedTime");
  
  transformMatrixLocation = glGetUniformLocation(gl2Program, "transformMatrix");

  mouseLocationx = glGetUniformLocation(gl2Program,"mousex");

  mouseLocationy = glGetUniformLocation(gl2Program,"mousey");

  mouseFlagLocation = glGetUniformLocation(gl2Program,"mouseFlag");

  vLocation = glGetUniformLocation(gl2Program, "velocity");

  ambLocation = glGetUniformLocation(gl2Program,"ambient");

  motionLocation = glGetAttribLocation(gl2Program,"motion");
  
  pointLocation = glGetAttribLocation(gl2Program, "point");
  
  //視野変換行列を求める
  modelviewMatrix.loadIdentity();
  modelviewMatrix.lookat(0.0f, 0.0f, 300.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
  
  //位置情報用のvboセットの初期設定
  glGenBuffers(sizeof(buffer)/sizeof(buffer[0]), buffer);
  
  glBindBuffer(GL_ARRAY_BUFFER, buffer[0]);
  
  glBufferData(GL_ARRAY_BUFFER, sizeof (GLfloat[3]) * POINTS, 0, GL_STATIC_DRAW);
  
  GLfloat (*point)[3] = (GLfloat (*)[3])glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
  
  for (int i = 0; i < POINTS; ++i) {
    point[i][0] = (2.0f * (float)rand() / (float)RAND_MAX - 1.0f)*60.0;
    point[i][1] = (2.0f * (float)rand() / (float)RAND_MAX - 1.0f)*60.0;
    point[i][2] = (2.0f * (float)rand() / (float)RAND_MAX - 1.0f)*60.0;
  }
  
  glUnmapBuffer(GL_ARRAY_BUFFER);

  glBindBuffer(GL_ARRAY_BUFFER, buffer[1]);
  
  glBufferData(GL_ARRAY_BUFFER, sizeof (GLfloat[3]) * POINTS, 0, GL_DYNAMIC_COPY);
  
  //速度情報用のvboの初期設定
  glBindBuffer(GL_ARRAY_BUFFER, buffer[2]);
  
  glBufferData(GL_ARRAY_BUFFER, sizeof (GLfloat[3]) * POINTS, 0, GL_STATIC_DRAW);
  
  GLfloat (*motion)[3] = (GLfloat (*)[3])glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
  
  for (int i = 0; i < POINTS; ++i) {
    motion[i][0] = 0.0f;
    motion[i][1] = 0.0f;
    motion[i][2] = 0.0f;
  }
  
  glUnmapBuffer(GL_ARRAY_BUFFER);
  
  glBindBuffer(GL_ARRAY_BUFFER, buffer[3]);
  
  glBufferData(GL_ARRAY_BUFFER, sizeof (GLfloat[3]) * POINTS, 0, GL_STATIC_DRAW);
  
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  
  trackball.initialize();

  glutGet(GLUT_ELAPSED_TIME);
  
  glClearColor(0.0, 0.0, 0.0, 1.0);
}

int main(int argc, char *argv[])
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
  glutCreateWindow(argv[0]);
  glutDisplayFunc(display);
  glutReshapeFunc(resize);
  glutIdleFunc(idle);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutKeyboardFunc(keyboard);
  init();
  glutMainLoop();
  glDeleteBuffers(4,buffer);

  return 0;
}
