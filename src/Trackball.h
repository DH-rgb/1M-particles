#ifndef TRACKBALL_H
#define TRACKBALL_H

class Trackball {
  int cx, cy;                   // �ɥ�å�����
  float sx, sy;                 // �ޥ����ΰ���
  float cq[4];                  // ��ž�ν����
  float tq[4];                  // �ɥ�å���β�ž
  float array[16];              // ��ž���Ѵ�����
  bool drag;                    
public:
  Trackball() {};
  virtual ~Trackball() {};
  void initialize();            // �����
  void region(int w, int h);    // �ȥ�å��ܡ���������ϰ�
  void start(int x, int y);     
  void motion(int x, int y);    // ��ž���Ѵ�����׻�
  void stop(int x, int y);      
  const float *get() const { return array; } // ������Ф�
};

#endif