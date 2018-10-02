
#pragma once


struct rect
{
  int x, y, w, h;

  bool empty() { return (w <= 0) || (h <= 0); }
};

class rect_packer
{
public:
  typedef std::vector<rect> rect_array;

public:
  rect_packer(int w, int h);
  rect add(rect);

  void clear();

private:
  rect get_fit_rect(rect);
  void update_free_rect(const rect& r);

private:
  int width_;
  int height_;
  rect_array rect_array_;
  rect_array free_rect_array_;
};

