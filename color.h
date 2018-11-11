
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
};

typedef vec<float, 4, color_mem> color;

