
#pragma once;


struct color_base
{
  static const int N = 4;
  typedef float value_t;
  union {
    struct {
      float r, g, b, a;
    };
    float v[N];
  };
  color_base() {}
  color_base(float r, float g, float b, float a)
    : r(r), g(g), b(b), a(a)
  {}
  color_base(const color_base& o)
    : r(o.r), g(o.g), b(o.b), a(o.a)
  {}
};

typedef vec<color_base> color;

