
#pragma once

struct quarternion_base
{
  static const int N = 4;
  typedef float value_t;
  quarternion_base() {}
  quarternion_base(float x, float y, float z, float w)
    : x(x), y(y), z(z), w(w) {}
  quarternion_base(const quarternion_base& o)
    : quarternion_base(o.x, o.y, o.z, o.w) {}
  union {
    struct {
      float x, y, z, w;
    };
    struct {
      vec3 v;
      float w;
    };
    float q[4];
  };
};

struct quarternion : vec<quarternion_base>
{
  typedef vec<quarternion_base> base_t;

  quarternion() {}
  quarternion(const base_t& v) : base_t(v){}
  quarternion(float x, float y, float z, float w) : base_t(x, y, z, w) {}
  quarternion(std::initializer_list<float> init)
    : quarternion(init.begin()[0], init.begin()[1], init.begin()[2], init.begin()[3])
  {}
  quarternion(const vec3& v, float w)
    : quarternion(v.x, v.y, v.z, w) {}
  quarternion(const quarternion& o)
    : quarternion(o.x, o.y, o.z, o.w) {}

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

inline quarternion conj(const quarternion& q) { return quarternion(-q.v, q.w); }
inline quarternion inverse(const quarternion& q) { return conj(q) / lenq(q); }
inline vec3 rotate_vec3(const quarternion& q, const vec3& v)
{
  vec3 t = 2 * cross(q.v, v);
  return v + q.w * t + cross(q.v, t);
}
inline vec4 rotate_vec4(const quarternion& q, const vec4& v)
{
  vec3 t = rotate_vec3(q, as_vec3(v));
  return vec4(t.x, t.y, t.z, v.w);
}

inline matrix to_matrix(const quarternion& q)
{
  matrix m;

  float a = q.w;
  float b = q.x;
  float c = q.y;
  float d = q.z;
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
