
#pragma once

struct matrix
{
  union
  {
    struct
    {
      float _00, _10, _20, _30,
            _01, _11, _21, _31,
            _02, _12, _22, _32,
            _03, _13, _23, _33;
    };
    float m[16];
    float a[4][4];
  };
  matrix() {};
  matrix(float _00, float _01, float _02, float _03,
         float _10, float _11, float _12, float _13,
         float _20, float _21, float _22, float _23,
         float _30, float _31, float _32, float _33)
    : _00(_00), _10(_10), _20(_20), _30(_30),
      _01(_01), _11(_11), _21(_21), _31(_31),
      _02(_02), _12(_12), _22(_22), _32(_32),
      _03(_03), _13(_13), _23(_23), _33(_33) {}
  matrix(float *m)
    : _00(m[ 0]), _10(m[ 1]), _20(m[ 2]), _30(m[ 3]),
      _01(m[ 4]), _11(m[ 5]), _21(m[ 6]), _31(m[ 7]),
      _02(m[ 8]), _12(m[ 9]), _22(m[10]), _32(m[11]),
      _03(m[12]), _13(m[13]), _23(m[14]), _33(m[15]) {}
  matrix(const matrix& o) { std::memcpy(m, o.m, sizeof(float) * 16); };
  matrix& operator=(const matrix& o)
  {
    std::memcpy(m, o.m, sizeof(float) * 16);
    return *this;
  }

  float& operator[](int i) { return m[i]; }
  float operator[](int i) const { return m[i]; }

  matrix& operator+=(const matrix& b)
  {
    for (int i=0; i<16; ++i) { m[i] += b.m[i]; }
    return *this;
  }
  matrix& operator-=(const matrix& b)
  {
    for (int i=0; i<16; ++i) { m[i] -= b.m[i]; }
    return *this;
  }
  matrix& operator*=(float b)
  {
    for (int i=0; i<16; ++i) { m[i] *= b; }
    return *this;
  }
  matrix& operator/=(float b)
  {
    for (int i=0; i<16; ++i) { m[i] /= b; }
    return *this;
  }

  static matrix identity()
  {
    return
      matrix(1, 0, 0, 0,
             0, 1, 0, 0,
             0, 0, 1, 0,
             0, 0, 0, 1);
  }
  static matrix ortho(float l, float r, float b, float t, float n, float f);
  static matrix perspective(float y_fov, float aspect, float n, float f);
  static matrix look_at(vec3 eye, vec3 center, vec3 up);
  static matrix rotate_x(float angle);
  static matrix rotate_y(float angle);
  static matrix rotate_z(float angle);
  static matrix rotate_axis(const vec3& axis, float angle);
};
inline matrix operator-(const matrix& a) {
  return matrix(-a._00, -a._01, -a._02, -a._03,
                -a._10, -a._11, -a._12, -a._13,
                -a._20, -a._21, -a._22, -a._23,
                -a._30, -a._31, -a._32, -a._33);
}
inline matrix operator+(const matrix& a, const matrix& b)
{
  matrix r(a); r += b; return r;
}
inline matrix operator-(const matrix& a, const matrix& b)
{
  matrix r(a); r -= b; return r;
}
inline matrix operator*(const matrix& a, const float b)
{
  matrix r(a); r *= b; return r;
}
inline matrix operator*(float b, const matrix& a)
{
  matrix r(a); r *= b; return r;
}
inline matrix operator/(const matrix& a, const float b)
{
  matrix r(a); r /= b; return r;
}
inline matrix concat(const matrix& a, const matrix& b)
{
  matrix r;
  for (int i=0; i<16; ++i) {
    r.m[i] = 0;
    int row = i % 4;
    int col = i / 4;
    for (int j=0; j<4; ++j) {
      r.m[i] += a.m[row+4*j] * b.m[col*4+j];
    }
  }
  return r;
}
inline vec3 transform_point(const matrix& m, const vec3& v)
{
  float w = m.m[3]*v.x + m.m[7]*v.y + m.m[11]*v.z + m.m[15];
  return vec3(
    (m.m[ 0]*v.x + m.m[ 4]*v.y + m.m[ 8]*v.z + m.m[12]) / w,
    (m.m[ 1]*v.x + m.m[ 5]*v.y + m.m[ 9]*v.z + m.m[13]) / w,
    (m.m[ 2]*v.x + m.m[ 6]*v.y + m.m[10]*v.z + m.m[14]) / w);
}
inline vec3 transform_direction(const matrix& m, const vec3& v)
{
  return vec3(
    m.m[ 0]*v.x + m.m[ 4]*v.y + m.m[ 8]*v.z,
    m.m[ 1]*v.x + m.m[ 5]*v.y + m.m[ 9]*v.z,
    m.m[ 2]*v.x + m.m[ 6]*v.y + m.m[10]*v.z);
}
inline vec4 transform(const matrix& m, const vec4& v)
{
  return vec4(
    m.m[ 0]*v.x + m.m[ 4]*v.y + m.m[ 8]*v.z + m.m[12]*v.w,
    m.m[ 1]*v.x + m.m[ 5]*v.y + m.m[ 9]*v.z + m.m[13]*v.w,
    m.m[ 2]*v.x + m.m[ 6]*v.y + m.m[10]*v.z + m.m[14]*v.w,
    m.m[ 3]*v.x + m.m[ 7]*v.y + m.m[11]*v.z + m.m[15]*v.w);
}

inline matrix matrix::ortho(float l, float r, float b, float t, float n, float f)
{
  matrix m;

  m.a[0][0] = 2.f/(r-l);
  m.a[0][1] = m.a[0][2] = m.a[0][3] = 0.f;

  m.a[1][1] = 2.f/(t-b);
  m.a[1][0] = m.a[1][2] = m.a[1][3] = 0.f;

  m.a[2][2] = -2.f/(f-n);
  m.a[2][0] = m.a[2][1] = m.a[2][3] = 0.f;

  m.a[3][0] = -(r+l)/(r-l);
  m.a[3][1] = -(t+b)/(t-b);
  m.a[3][2] = -(f+n)/(f-n);
  m.a[3][3] = 1.f;

  return m;
}
inline matrix matrix::perspective(float y_fov, float aspect, float n, float f)
{
  /* NOTE: Degrees are an unhandy unit to work with.
   * linmath.h uses radians for everything! */
  float const a = 1.f / (float) tan(y_fov / 2.f);

  matrix m;

  m.a[0][0] = a / aspect;
  m.a[0][1] = 0.f;
  m.a[0][2] = 0.f;
  m.a[0][3] = 0.f;

  m.a[1][0] = 0.f;
  m.a[1][1] = a;
  m.a[1][2] = 0.f;
  m.a[1][3] = 0.f;

  m.a[2][0] = 0.f;
  m.a[2][1] = 0.f;
  m.a[2][2] = f / (n - f);
  m.a[2][3] = -1.f;

  m.a[3][0] = 0.f;
  m.a[3][1] = 0.f;
  m.a[3][2] = -(f * n) / (f - n);
  m.a[3][3] = 0.f;

  return m;
}
inline matrix matrix::look_at(vec3 eye, vec3 center, vec3 up)
{
  vec3 f = normalize(center - eye);
  vec3 s = normalize(cross(f, up));
  vec3 u = cross(s, f);

  matrix m;
  m.a[0][0] = s.x;
  m.a[1][0] = s.y;
  m.a[2][0] = s.z;
  m.a[3][0] = 0.f;

  m.a[0][1] = u.x;
  m.a[1][1] = u.y;
  m.a[2][1] = u.z;
  m.a[3][1] = 0.f;

  m.a[0][2] =-f.x;
  m.a[1][2] =-f.y;
  m.a[2][2] =-f.z;
  m.a[3][2] = 0.f;

  m.a[0][3] = 0.f;
  m.a[1][3] = 0.f;
  m.a[2][3] = 0.f;
  
  m.a[3][0] =-dot(s, eye);
  m.a[3][1] =-dot(u, eye);
  m.a[3][2] = dot(f, eye);
  m.a[3][3] = 1.f;

  return m;
}

inline matrix matrix::rotate_x(float angle)
{
  float s = sinf(angle);
  float c = cosf(angle);
  return matrix(
    1.f, 0.f, 0.f, 0.f,
    0.f,   c,   s, 0.f,
    0.f,  -s,   c, 0.f,
    0.f, 0.f, 0.f, 1.f);
}
inline matrix matrix::rotate_y(float angle)
{
  float s = sinf(angle);
  float c = cosf(angle);
  return matrix(
      c, 0.f,   s, 0.f,
    0.f, 1.f, 0.f, 0.f,
     -s, 0.f,   c, 0.f,
    0.f, 0.f, 0.f, 1.f);
}
inline matrix matrix::rotate_z(float angle)
{
  float s = sinf(angle);
  float c = cosf(angle);
  return matrix(
       c,   s, 0.f, 0.f,
      -s,   c, 0.f, 0.f,
     0.f, 0.f, 1.f, 0.f,
     0.f, 0.f, 0.f, 1.f);
}
inline matrix matrix::rotate_axis(const vec3& v, float angle)
{
  const float c = cos(angle);
  const float invc = 1 - c;
  const float s = sin(angle);
  return matrix(
    v.x*v.x*invc + c,     v.x*v.y*invc + v.z*s, v.z*v.x*invc - v.y*s, 0.f,
    v.x*v.y*invc - v.z*s, v.y*v.y*invc + c,     v.z*v.y*invc + v.x*s, 0.f,
    v.x*v.z*invc + v.y*s, v.y*v.z*invc - v.x*s, v.z*v.z*invc + c,     0.f,
    0.f,                  0.f,                  0.f,                  1.f);

}

inline const float *as_array(const matrix& m) { return m.m; }

