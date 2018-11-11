
#include "stdafx.h"

#include "trackball.h"


trackball::trackball(const vec3& c, float r)
  : cur_(quarternion::identity()), rot_(quarternion::identity()),
    center_(c), radius_(r),
    button_(-1), prev_pos_{}
{
}

void trackball::on_mouse_button(GLFWwindow *window, int button, int action, int mod)
{
  if (action == GLFW_PRESS) {
    button_ = button;
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
      cur_ = quarternion::identity();
      double x, y;
      glfwGetCursorPos(window, &x, &y);
      int width, height;
      glfwGetWindowSize(window, &width, &height);
      vec2 pos{ (float)(x * 2 - width) / width, (float)(height - 2 * y) / height };
      prev_pos_ = unproject(pos);
    }
  } else {
    button_ = -1;
    rot_ *= cur_;
    cur_ = quarternion::identity();
  }
}

void trackball::on_cursor_move(GLFWwindow *window, double x, double y)
{
  switch (button_) {
  case GLFW_MOUSE_BUTTON_RIGHT:
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    vec2 cur_pos{ (float)(x * 2 - width) / width, (float)(height - 2 * y) / height };
    vec3 cur = unproject(cur_pos);
    vec3 axis = normalize(cross(prev_pos_, cur));
    float c = dot(prev_pos_, cur);
    cur_ = quarternion::rotate(axis, std::acosf(c));
    break;
  }
}

void trackball::on_cursor_enter(GLFWwindow*, int enter)
{
  if (enter == GLFW_FALSE) {
    button_ = -1;
  }
}

void trackball::on_scroll(GLFWwindow*, double xoffset, double yoffset)
{
  radius_ += 1.f * float(yoffset);
  radius_ = std::max(radius_, 0.1f);
}

vec3 trackball::unproject(vec2 v)
{
  float d = 1.f - lenq(v);
  if (d < 0.f) {
    v = normalize(v);
    return vec3{ v.x, v.y, 0.f };
  } else {
    return vec3{ v.x, v.y, std::sqrtf(d) };
  }
}

matrix trackball::get_matrix()
{
  return to_matrix(rot_ * cur_);
}

