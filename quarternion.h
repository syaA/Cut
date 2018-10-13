
#pragma once

struct quarternion
{
  vec3 v;
  float w;

  quarternion() {}
  quarternion(const vec3& v, float w) : v(v), w(w) {}
  quarternion(float x, float y, float z, float w) : quarternion(vec3(x, y, z), w) {}
  quarternion(std::initializer_list<float> init)
    : quarternion(init.begin()[0], init.begin()[1], init.begin()[2], init.begin()[3])
  {}
  quarternion(const quarternion& o)
    : quarternion(o.v, o.w) {}

  quarternion& operator/=(float f)
  {
    v /= f;
    w /= f;
    return *this;
  }
  quarternion& operator*=(const quarternion& o)
  {
    float tw = w;
    w = tw * o.w - dot(v, o.v);
    v = cross(v, o.v) + o.w * v + tw * o.v;
    return *this;
  }
  
  static quarternion identity();
  static quarternion rotate(const vec3& axis, float rad);
};

inline quarternion quarternion::identity()
{
  return quarternion(0.f, 0.f, 0.f, 1.f);
}

inline quarternion quarternion::rotate(const vec3& axis, float rad)
{
  return quarternion(axis * std::sinf(rad/2), cos(rad/2));
}

inline quarternion operator*(const quarternion& a, const quarternion& b)
{
  quarternion r(a); r *= b; return r;
}
inline quarternion operator/(const quarternion& a, float b)
{
  quarternion r(a); r /= b; return r;
}

inline quarternion conj(const quarternion& q) { return quarternion(-q.v, q.w); }
inline quarternion inverse(const quarternion& q) { return conj(q) / (lenq(q.v) + q.w * q.w); }
inline vec3 rotate(const quarternion& q, const vec3& v)
{
  vec3 t = 2.f * cross(q.v, v);
  return v + q.w * t + cross(q.v, t);
}
inline vec4 rotate(const quarternion& q, const vec4& v)
{
  vec3 t = rotate(q, as_vec3(v));
  return vec4(t.x, t.y, t.z, v.w);
}

inline matrix to_matrix(const quarternion& q)
{
  matrix m;

  float a = q.w;
  float b = q.v.x;
  float c = q.v.y;
  float d = q.v.z;
  float a2 = a*a;
  float b2 = b*b;
  float c2 = c*c;
  float d2 = d*d;

  m.a[0][0] = a2 + b2 - c2 - d2;
  m.a[0][1] = 2.f*(b*c + a*d);
  m.a[0][2] = 2.f*(b*d - a*c);
  m.a[0][3] = 0.f;

  m.a[1][0] = 2*(b*c - a*d);
  m.a[1][1] = a2 - b2 + c2 - d2;
  m.a[1][2] = 2.f*(c*d + a*b);
  m.a[1][3] = 0.f;

  m.a[2][0] = 2.f*(b*d + a*c);
  m.a[2][1] = 2.f*(c*d - a*b);
  m.a[2][2] = a2 - b2 - c2 + d2;
  m.a[2][3] = 0.f;

  m.a[3][0] = m.a[3][1] = m.a[3][2] = 0.f;
  m.a[3][3] = 1.f;

  return m;
}
