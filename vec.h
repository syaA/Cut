
#pragma once

template<class ValueT, int N>
struct vec_mem {};

template<class ValueT>
struct vec_mem<ValueT, 2>
{
  typedef ValueT value_t;
  value_t x, y;
  value_t *begin() { return &x; }
  const value_t *begin() const { return &x; }
  const value_t *end() const { return &y + 1; }

  vec_mem() {}
  vec_mem(value_t x, value_t y) : x(x), y(y) {}
  explicit vec_mem(value_t v) : vec_mem(v, v) {}
  vec_mem(const vec_mem& o) : vec_mem(o.x, o.y) {}
};

template<class ValueT>
struct vec_mem<ValueT, 3>
{
  typedef ValueT value_t;
  value_t x, y, z;
  value_t *begin() { return &x; }
  const value_t *begin() const { return &x; }
  const value_t *end() const { return &z + 1; }

  vec_mem() {}
  vec_mem(value_t x, value_t y, value_t z) : x(x), y(y), z(z) {}
  explicit vec_mem(value_t v) : vec_mem(v, v, v) {}
  vec_mem(const vec_mem& o) : vec_mem(o.x, o.y, o.z) {}
};

template<class ValueT>
struct vec_mem<ValueT, 4>
{
  typedef ValueT value_t;
  value_t x, y, z, w;
  value_t *begin() { return &x; }
  const value_t *begin() const { return &x; }
  const value_t *end() const { return &w + 1; }

  vec_mem() {}
  vec_mem(value_t x, value_t y, value_t z, value_t w) : x(x), y(y), z(z), w(w) {}
  explicit vec_mem(value_t v) : vec_mem(v, v, v) {}
  vec_mem(const vec_mem& o) : vec_mem(o.x, o.y, o.z, o.w) {}
};


template<class ValueT, int Dim, template<class, int> class MemberT>
struct vec : public MemberT<ValueT, Dim>
{
  typedef MemberT<ValueT, Dim> member_t;
  typedef ValueT value_t;
  static const int N = Dim;

  vec() {}
  explicit vec(float v) : member_t(v) {}
  template<class... Args>
  vec(Args... args) : member_t(args...) {}
  vec(const vec& o) : member_t(o) {}
  vec(std::initializer_list<value_t> init) : member_t(value_t()) { std::copy(init.begin(), init.end(), begin()); }

  value_t *begin() { return member_t::begin(); }
  const value_t *begin() const { return member_t::begin(); }
  const value_t *end() const { return member_t::end(); }

  value_t operator[](int i) const { return begin()[i]; }
  value_t& operator[](int i) { return begin()[i]; }

  template<class Pred>
  vec& map(Pred p) { for (auto& v : *this) { v = p(v); } return *this; }
  template<class Pred>
  vec& zip(const vec& o, Pred p) { for (int i=0; i<N; ++i) { (*this)[i] = p((*this)[i], o[i]); } return *this; }
  template<class Pred>
  float reduce(Pred p, value_t r = 0.f) { for (int i=0; i<N; ++i) { p(r, (*this)[i]); } return r; }
  
  vec& operator=(const vec& v) { return zip(v, [](float a, float b) { return b; }); }
  vec& operator+=(const vec& v) { return zip(v, [](float a, float b) { return a + b; }); }
  vec& operator-=(const vec& v) { return zip(v, [](float a, float b) { return a - b; }); }
  vec& operator*=(const vec& v) { return zip(v, [](float a, float b) { return a * b; }); }
  vec& operator/=(const vec& v) { return zip(v, [](float a, float b) { return a / b; }); }
  vec& operator*=(float a) { return map([=](float v) { return v * a; }); }
  vec& operator/=(float a) { return map([=](float v) { return v / a; }); }

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

template<class T, int Dim, template<class, int> class MemberT>
vec<T, Dim, MemberT>
operator-(const vec<T, Dim, MemberT>& v)
{
  return v * (T)-1;
}

template<class T, int Dim, template<class, int> class MemberT>
vec<T, Dim, MemberT>
operator+(const vec<T, Dim, MemberT>& a, const vec<T, Dim, MemberT>& b)
{
  vec<T, Dim, MemberT> r(a); r += b; return r;
}

template<class T, int Dim, template<class, int> class MemberT>
vec<T, Dim, MemberT>
operator-(const vec<T, Dim, MemberT>& a, const vec<T, Dim, MemberT>& b)
{
  vec<T, Dim, MemberT> r(a); r -= b; return r;
}

template<class T, int Dim, template<class, int> class MemberT>
vec<T, Dim, MemberT>
operator*(const vec<T, Dim, MemberT>& a, const T b)
{
  vec<T, Dim, MemberT> r(a); r *= b; return r;
}

template<class T, int Dim, template<class, int> class MemberT>
vec<T, Dim, MemberT>
operator*(T b, const vec<T, Dim, MemberT>& a)
{
  vec<T, Dim, MemberT> r(a); r *= b; return r;
}

template<class T, int Dim, template<class, int> class MemberT>
vec<T, Dim, MemberT>
operator*(const vec<T, Dim, MemberT>& a, const vec<T, Dim, MemberT>& b)
{
  vec<T, Dim, MemberT> r(a); r *= b; return r;
}

template<class T, int Dim, template<class, int> class MemberT>
vec<T, Dim, MemberT>
operator/(const vec<T, Dim, MemberT>& a, const T b)
{
  vec<T, Dim, MemberT> r(a); r /= b; return r;
}

template<class T, int Dim, template<class, int> class MemberT>
vec<T, Dim, MemberT>
operator/(const vec<T, Dim, MemberT>& a, const vec<T, Dim, MemberT>& b)
{
  vec<T, Dim, MemberT> r(a); r /= b; return r;
}

template<class T, int Dim, template<class, int> class MemberT>
T
dot(const vec<T, Dim, MemberT>& a, const vec<T, Dim, MemberT>& b)
{
  return (a * b).reduce([](T& r, T a) { r += a; });
}

template<class T, int Dim, template<class, int> class MemberT>
T
lenq(const vec<T, Dim, MemberT>& v)
{
  return dot(v, v);
}

template<int Dim, template<class, int> class MemberT>
float
len(const vec<float, Dim, MemberT>& v)
{
  return std::sqrtf(lenq(v));
}

template<class T, int Dim, template<class, int> class MemberT>
vec<T, Dim, MemberT>
normalize(const vec<T, Dim, MemberT>& v)
{
  return v / len(v);
}

template<class T, int Dim, template<class, int> class MemberT>
T *
as_array(vec<T, Dim, MemberT>& v)
{
  return v.begin();
}

template<class T, int Dim, template<class, int> class MemberT>
const T *as_array(const vec<T, Dim, MemberT>& v)
{
  return v.begin();
}

template<class T, int Dim, template<class, int> class MemberT>
vec<T, Dim, MemberT>
min(const vec<T, Dim, MemberT>& a, const vec<T, Dim, MemberT>& b)
{
  vec<T, Dim, MemberT> r(a); r.zip(std::min); return r;
}

template<class T, int Dim, template<class, int> class MemberT>
vec<T, Dim, MemberT>
max(const vec<T, Dim, MemberT>& a, const vec<T, Dim, MemberT>& b)
{
  vec<T, Dim, MemberT> r(a); r.zip(std::max); return r;
}

template<class T, int Dim, template<class, int> class MemberT>
vec<T, Dim, MemberT>
clamp(const vec<T, Dim, MemberT>& a, const vec<T, Dim, MemberT>& mn, const vec<T, Dim, MemberT>& mx)
{
  vec<T, Dim, MemberT> r(a);
  for (int i=0; i<vec<T, Dim, MemberT>::N; ++i) {
    r[i] = std::min(std::max(r[i], mn[i]), mx[i]);
  }
  return r;
}

template<class CharT, class TraitsT, class T, int Dim, template<class, int> class MemberT>
std::basic_ostream<CharT, TraitsT>&
operator<<(std::basic_ostream<CharT, TraitsT>& o, const vec<T, Dim, MemberT>& v) {
  const int dim = vec<T, Dim, MemberT>::N;
  o << "(";
  for (int i=0; i<dim-1; ++i) {
    o << v[i] << ",";
  }
  o << v[dim-1] << ")";
  return o;
}

template<class T, template<class, int> class MemberT>
T
cross(const vec<T, 2, MemberT>& a, const vec<T, 2, MemberT>& b)
{
  return a.x*b.y-a.y*b.x;
}

template<class T, template<class, int> class MemberT>
vec<T, 3, MemberT>
cross(const vec<T, 3, MemberT>& a, const vec<T, 3, MemberT>& b)
{
  return vec<T, 3, MemberT>(a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x);
}

template<class T, template<class, int> class MemberT>
vec<T, 4, MemberT>
cross(const vec<T, 4, MemberT>& a, const vec<T, 4, MemberT>& b)
{
  return vec<T, 4, MemberT>(a.y*b.z-a.z*b.y, a.z*b.x-a.x*b.z, a.x*b.y-a.y*b.x, 0.0f);
}

template<class T, template<class, int> class MemberT>
vec<T, 3, MemberT>
rotate(const vec<T, 3, MemberT>& v, const vec<T, 3, MemberT>& n, T c, T s)
{
  return n * dot(n, v) + (v - n * dot(n, v)) * c - cross(v, n) * s;
}

template<class T, template<class, int> class MemberT>
const vec<T, 3, MemberT>&
as_vec3(const vec<T, 4, MemberT>& v)
{
  return *(const vec<T, 3, MemberT>*)(&v);
}


typedef vec<float, 2, vec_mem> vec2;
typedef vec<float, 3, vec_mem> vec3;
typedef vec<float, 4, vec_mem> vec4;
typedef vec<int, 2, vec_mem> ivec2;
typedef vec<int, 3, vec_mem> ivec3;
typedef vec<int, 4, vec_mem> ivec4;
typedef vec<unsigned, 2, vec_mem> uvec2;
typedef vec<unsigned, 3, vec_mem> uvec3;
typedef vec<unsigned, 4, vec_mem> uvec4;


