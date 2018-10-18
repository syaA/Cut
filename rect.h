
#pragma once

template<class T>
struct basic_rect
{
  typedef T value_t;
  value_t x, y, w, h;

  bool empty() { return (w <= value_t(0)) || (h <= value_t(0)); }
};

// a が b に完全に含まれるか？
template<class T>
bool is_include_rect(const basic_rect<T>& a, const basic_rect<T>& b)
{
  return
    (a.x >= b.x) &&
    ((a.x + a.w) <= (b.x + b.w)) &&
    (a.y >= b.y) &&
    ((a.y + a.h) <= (b.y + b.h));
}

// 矩形の交差を得る
template<class T>
basic_rect<T> intersect_rect(const basic_rect<T>& a, const basic_rect<T>& b)
{
  basic_rect<T> r{0, 0, 0, 0};
  if (((a.x + a.w) < b.x) ||
      (a.x > (b.x + b.w)) ||
      ((a.y + a.h) < b.y) ||
      (a.y > (b.y + b.h))) {
    // 交差無し
    return r;
  }

  r.x = (a.x < b.x) ? b.x : a.x;
  r.y = (a.y < b.y) ? b.y : a.y;
  r.w = (((a.x + a.w) > (b.x + b.w)) ? (b.x + b.w) : (a.x + a.w)) - r.x;
  r.h = (((a.y + a.h) > (b.y + b.h)) ? (b.y + b.h) : (a.y + a.h)) - r.y;

  return r;
}


typedef basic_rect<int> irect;
typedef basic_rect<float> rect;
