
#pragma once

#include "figure.h"


class trackball
{
public:
  typedef std::shared_ptr<trackball> ptr_t;

public:
  trackball(const vec3& c, float r);

  void on_mouse_button(GLFWwindow*, int, int, int);
  void on_cursor_move(GLFWwindow*, double, double);
  void on_cursor_enter(GLFWwindow*, int);
  void on_scroll(GLFWwindow*, double, double);

  matrix get_matrix();

private:
  vec3 unproject(vec2);

private:
  quarternion rot_;
  quarternion cur_;
  
  vec3 center_;
  float radius_;

  int button_;
  vec3 prev_pos_;
};
