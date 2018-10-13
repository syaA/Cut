
#pragma once;

template<class ValueT, int N>
struct color_mem {};

template<class ValueT>
struct color_mem<ValueT, 4>
{
  typedef ValueT value_t;
  float r, g, b, a;
  value_t *begin() { return &r; }
  const value_t *begin() const { return &r; }
  const value_t *end() const { return &a + 1; }

  color_mem() {}
  color_mem(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
  explicit color_mem(float v) : color_mem(v, v, v, v) {}
  color_mem(const color_mem& o) : color_mem(o.r, o.g, o.b, o.a) {}
};

typedef vec<float, 4, color_mem> color;

