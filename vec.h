
#pragma once

template<class ValueT>
struct vec2_base
{
  static const int N = 2;
  typedef ValueT value_t;
  union {
    struct {
      value_t x, y;
    };
    value_t v[N];
  };
  vec2_base() {}
  explicit vec2_base(value_t v) : x(v), y(v) {}
  vec2_base(value_t x, value_t y)
    : x(x), y(y) {}
  vec2_base(std::initializer_list<value_t> init)
    : vec2_base(value_t(), value_t())
  {
    value_t *d = v;
    for (auto f : init) {
      *d++ = f;
    }
  }
  vec2_base(const vec2_base& o)
    : x(o.x), y(o.y) {}
};

template<class ValueT>
struct vec3_base
{
  static const int N = 3;
  typedef ValueT value_t;
  union {
    struct {
      value_t x, y, z;
    };
    value_t v[N];
  };
  vec3_base() {}
  explicit vec3_base(value_t v) : x(v), y(v), z(v) {}
  vec3_base(value_t x, value_t y, value_t z)
    : x(x), y(y), z(z) {}
  vec3_base(std::initializer_list<value_t> init)
    : vec3_base(value_t(), value_t())
  {
    value_t *d = v;
    for (auto f : init) {
      *d++ = f;
    }
  }
  vec3_base(const vec3_base& o)
    : x(o.x), y(o.y), z(o.z) {}
};

template<class ValueT>
struct vec4_base
{
  static const int N = 4;
  typedef ValueT value_t;
  union {
    struct {
      value_t x, y, z, w;
    };
    value_t v[N];
  };
  vec4_base() {}
  explicit vec4_base(value_t v) : x(v), y(v), z(v), w(v) {}
  vec4_base(value_t x, value_t y, value_t z, value_t w)
    : x(x), y(y), z(z), w(w) {}
  vec4_base(std::initializer_list<value_t> init)
    : vec4_base(value_t(), value_t())
  {
    value_t *d = v;
    for (auto f : init) {
      *d++ = f;
    }
  }
  vec4_base(const vec4_base& o)
    : x(o.x), y(o.y), z(o.z), w(o.w) {}
};


template<class BaseT>
struct vec : public BaseT
{
  typedef BaseT base_t;
  typedef typename base_t::value_t value_t;
  static const int N = base_t::N;

  vec() {}
  explicit vec(float v) : base_t(v) {}
  template<class... Args>
  vec(Args... args) : base_t(args...) {}
  vec(const vec& o) : base_t(o) {}
  vec(std::initializer_list<value_t> init) : base_t(init) {}

  value_t operator[](int i) const { return base_t::v[i]; }
  value_t& operator[](int i) { return base_t::v[i]; }
  template<int Index>
  value_t get() { return base_t::v[Index]; }

  template<class Pred>
  vec& map(Pred p) {for (int i=0; i<N; ++i) { p(base_t::v[i]); } return *this; }
  template<class Pred>
  vec& zip(const vec& o, Pred p) { for (int i=0; i<N; ++i) { p(base_t::v[i], o[i]); } return *this; }
  template<class Pred>
  float reduce(Pred p, float r = 0.f) { for (int i=0; i<N; ++i) { p(r, base_t::v[i]); } return r; }
  
  vec& operator=(const vec& v) { return zip(v, [](float& a, float b) { a = b; }); }
  vec& operator+=(const vec& v) { return zip(v, [](float& a, float b) { a += b; }); }
  vec& operator-=(const vec& v) { return zip(v, [](float& a, float b) { a -= b; }); }
  vec& operator*=(const vec& v) { return zip(v, [](float& a, float b) { a *= b; }); }
  vec& operator/=(const vec& v) { return zip(v, [](float& a, float b) { a /= b; }); }
  vec& operator*=(float a) { return map([=](float& v) { v *= a; }); }
  vec& operator/=(float a) { return map([=](float& v) { v /= a; }); }

  bool operator==(const vec& b) {
    for (int i=0; i<N; ++i) {
      if ((*this)[i] != b[i]) { return false; }
    }
    return true;
  }
  bool operator!=(const vec& b) { return !(*this == b); }
  
  static vec replicate(float v) { return vec(v); }
  static vec zero() { return replicate(0); }
  static vec one() { return replicate(1); }
};

template<class BaseT>
vec<BaseT> operator-(const vec<BaseT>& v) { return v * -1; }
template<class BaseT>
vec<BaseT> operator+(const vec<BaseT>& a, const vec<BaseT>& b) { vec<BaseT> r(a); r += b; return r; }
template<class BaseT>
vec<BaseT> operator-(const vec<BaseT>& a, const vec<BaseT>& b) { vec<BaseT> r(a); r -= b; return r; }
template<class BaseT>
vec<BaseT> operator*(const vec<BaseT>& a, const typename BaseT::value_t b) { vec<BaseT> r(a); r *= b; return r; }
template<class BaseT>
vec<BaseT> operator*(typename BaseT::value_t b, const vec<BaseT>& a) { vec<BaseT> r(a); r *= b; return r; }
template<class BaseT>
vec<BaseT> operator*(const vec<BaseT>& a, const vec<BaseT>& b) { vec<BaseT> r(a); r *= b; return r; }
template<class BaseT>
vec<BaseT> operator/(const vec<BaseT>& a, const typename BaseT::value_t b) { vec<BaseT> r(a); r /= b; return r; }
template<class BaseT>
vec<BaseT> operator/(const vec<BaseT>& a, const vec<BaseT>& b) { vec<BaseT> r(a); r /= b; return r; }
template<class BaseT>
typename BaseT::value_t dot(const vec<BaseT>& a, const vec<BaseT>& b) { return (a * b).reduce([](typename BaseT::value_t& r, typename BaseT::value_t a) { r += a; }); }
template<class BaseT>
typename BaseT::value_t lenq(const vec<BaseT>& v) { return dot(v, v); }
template<class BaseT>
typename BaseT::value_t len(const vec<BaseT>& v) { return std::sqrtf(lenq(v)); }
template<class BaseT>
vec<BaseT> normalize(const vec<BaseT>& v) { return v / len(v); }
template<class BaseT>
typename BaseT::value_t *as_array(vec<BaseT>& v) { return v.v; }
template<class BaseT>
const typename BaseT::value_t *as_array(const vec<BaseT>& v) { return v.v; }
template<class BaseT>
vec<BaseT> min(const vec<BaseT>& a, const vec<BaseT>& b) { vec<BaseT> r(a); r.zip(std::min); return r; }
template<class BaseT>
vec<BaseT> max(const vec<BaseT>& a, const vec<BaseT>& b) { vec<BaseT> r(a); r.zip(std::max); return r; }
template<class BaseT>
vec<BaseT> clamp(const vec<BaseT>& a, const vec<BaseT>& mn, const vec<BaseT>& mx)
{
  vec<BaseT> r(a);
  for (int i=0; i<vec<BaseT>::N; ++i) {
    r[i] = std::min(std::max(r[i], mn[i]), mx[i]);
  }
  return r;
}

typedef vec<vec2_base<float>> vec2;
typedef vec<vec3_base<float>> vec3;
typedef vec<vec4_base<float>> vec4;

inline float cross(const vec2& a, const vec2& b)
{
  return a.x*b.y-a.y*b.x;
}

inline vec3 cross(const vec3& a, const vec3& b)
{
  return vec3(a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x);
}

inline vec4 cross(const vec4& a, const vec4& b)
{
  return vec4(a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x, 0.0f);
}

inline vec3 rotate(const vec3& v, const vec3& n, float c, float s)
{
  return n * dot(n, v) + (v - n * dot(n, v)) * c - cross(v, n) * s;
}

typedef vec<vec2_base<int>> ivec2;
typedef vec<vec3_base<int>> ivec3;
typedef vec<vec4_base<int>> ivec4;
typedef vec<vec2_base<unsigned>> uvec2;
typedef vec<vec3_base<unsigned>> uvec3;
typedef vec<vec4_base<unsigned>> uvec4;


template<class ValueT>
auto as_vec3(const vec<vec4_base<ValueT>>& v)
{
  typedef vec<vec3_base<ValueT>> v3_t;
  return (const v3_t&)(*v.v);
}
