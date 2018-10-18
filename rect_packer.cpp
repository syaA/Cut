
#include "stdafx.h"

#include "rect_packer.h"


rect_packer::rect_packer(int w, int h)
  : width_(w), height_(h)
{
  // フルサイズ空き矩形.
  free_rect_array_.push_back({0, 0, (float)w, (float)h});
}

rect rect_packer::add(rect r)
{
  // 空き矩形の中から最も良さそうなものを選ぶ。
  r = get_fit_rect(r);
  if (!r.empty()) {
    // 矩形を追加
    rect_array_.push_back(r);
    // 空き矩形を更新.
    update_free_rect(r);
  }

  return r;

}

rect rect_packer::get_fit_rect(rect r)
{
  // free_array から入れる場所を探す.
  // 幅と高さのどちらかだけでもピッタリのが良いことにする.
  float most_fit = std::numeric_limits<float>::max();
  for (const auto& free : free_rect_array_) {
    if ((free.w >= r.w) && (free.h >= r.h)) {
      float f = std::min(free.w - r.w, free.h - r.h);
      if (f <= 0.f) {
        // 終わり.
        r.x = free.x;
        r.y = free.y;
        return r;
      } else if (f < most_fit) {
        most_fit = f;
        r.x = free.x;
        r.y = free.y;
      }
    }
  }
  if ((most_fit > width_) || (most_fit > height_)) {
    r.w = r.h = 0;
  }
  return r;
}

void rect_packer::update_free_rect(const rect& r)
{
  rect_array new_free_rect_array;
  new_free_rect_array.reserve(free_rect_array_.size());
  for (const auto& free : free_rect_array_) {
    // 交差矩形
    rect ir = intersect_rect(free, r);
    if (ir.empty()) {
      // 追加する矩形と交わらないので、そのまま
      new_free_rect_array.push_back(free);
    } else {
      // 追加する矩形と交わるので、分割
      rect split[] = {
        // 左側
        {free.x, free.y, ir.x - free.x, free.h},
        // 右側
        {ir.x + ir.w, free.y, free.x + free.w - (ir.x + ir.w), free.h},
        // 上側
        {free.x, free.y, free.w, ir.y - free.y},
        // 下側
        {free.x, ir.y + ir.h, free.w, free.y + free.h - (ir.y + ir.h)},
      };
      // 完全に含まれるような空き矩形が既にあれば、マージする
      for (int i=0; i<4; ++i) {
        if (split[i].empty()) {
          continue;
        }
        bool need_push_back = true;
        for (auto it = new_free_rect_array.begin();
             it != new_free_rect_array.end();) {
          if (is_include_rect(*it, split[i])) {
            // 追加する矩形が既存の空き矩形を含む
            it = new_free_rect_array.erase(it);
            continue;
          } else if (is_include_rect(split[i], *it)) {
            // 既存の空き矩形が追加する矩形を含む
            need_push_back = false;
            break;
          }
          ++it;
        }
        if (need_push_back) {
          new_free_rect_array.push_back(split[i]);
        }
      }
    }
  }
  free_rect_array_.swap(new_free_rect_array);
}

void rect_packer::clear()
{
  rect_array_.clear();
  free_rect_array_.clear();
  free_rect_array_.push_back({0, 0, (float)width_, (float)height_});

}
