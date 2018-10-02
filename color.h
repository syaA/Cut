
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
  color_base(std::initializer_list<float> init)
    : color_base(0.f, 0.f, 0.f, 0.f)
  {
    float *d = v;
    for (auto f : init) {
      *d++ = f;
    }
  }
  color_base(const color_base& o)
    : r(o.r), g(o.g), b(o.b), a(o.a)
  {}
};

typedef vec<color_base> color;

