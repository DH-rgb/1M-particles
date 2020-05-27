#ifndef MATRIX_H
#define MATRIX_H

#include <cstring>

class Matrix {
  float array[16];
  void projection(float *c, const float *a, const float *b);
  void multiply(float *c, const float *a, const float *b);
public:
  Matrix() {}
  Matrix(const float *a) { load(a); }
  Matrix(const Matrix &m) { load(m); }
  virtual ~Matrix() {}
  Matrix &load(const float *a) { memcpy(array, a, sizeof array); return *this; }
  Matrix &load(const Matrix &m) { return load(m.array); }
  Matrix &loadIdentity();
  Matrix &projection(float *c, const float *v) { projection(c, array, v); return *this; }
  Matrix &multiply(const Matrix &m, const Matrix &n) { multiply(array, m.array, n.array); return *this; }
  Matrix &multiply(const float *a, const Matrix &m) { multiply(array, a, m.array); return *this; }
  Matrix &multiply(const Matrix &m, const float *a) { multiply(array, m.array, a); return *this; }
  Matrix &multiply(const float *a) { float t[16]; multiply(t, array, a); memcpy(array, t, sizeof array); return *this; }
  Matrix &multiply(const Matrix &m) { return multiply(m.array); }
  Matrix &operator=(const float *a) { return load(a); }
  Matrix &operator=(const Matrix &m) { return load(m); }
  Matrix operator*(const float *a) const { Matrix t; t.multiply(*this, a); return t; }
  Matrix operator*(const Matrix &m) const { Matrix t; t.multiply(*this, m); return t; }
  Matrix &operator*=(const float *a) { return multiply(a); }
  Matrix &operator*=(const Matrix &m) { return multiply(m); }
  Matrix &translate(float x, float y, float z, float w = 1.0f);
  Matrix &translate(const float *t) { return translate(t[0], t[1], t[2], t[3]); }
  Matrix &scale(float x, float y, float z, float w = 1.0f);
  Matrix &scale(const float *s) { return scale(s[0], s[1], s[2], s[3]); }
  Matrix &rotate(float x, float y, float z, float a);
  Matrix &rotate(const float *r) { return rotate(r[0], r[1], r[2], r[3]); }
  Matrix &lookat(float ex, float ey, float ez, float tx, float ty, float tz, float ux, float uy, float uz);
  Matrix &orthogonal(float left, float right, float bottom, float top, float near, float far);
  Matrix &perspective(float left, float right, float bottom, float top, float near, float far);
  Matrix &camera(float fovy, float aspect, float near, float far);
  Matrix &transpose(const Matrix &m);
  Matrix &transpose() { Matrix t; t.transpose(*this); *this = t; return *this; }
  Matrix &invert(const Matrix &m);
  Matrix &invert() { return invert(*this); }
  Matrix &normal(const Matrix &m);
  Matrix &normal() { Matrix t; t.normal(*this); *this = t; return *this; }
  const float *get() const { return array; }
};

#endif
