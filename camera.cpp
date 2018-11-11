
#include "stdafx.h"

#include "camera.h"

#include "util.h"


camera::camera()
  : eye_{ 0.f, 10.f, 0.f }, at_{ 0.f, 0.f, 0.f }, up_{ 0.f, 1.f, 0.f },
    fov_(deg2rad(45.f)), aspect_(1.f), z_near_(0.1f), z_far_(100.f)
{
}

camera::camera(const vec3& eye, const vec3& at, const vec3& up,
               float fov, float aspect, float z_near, float z_far)
  : eye_(eye), at_(at), up_(up),
    fov_(fov), aspect_(aspect), z_near_(z_near), z_far_(z_far)
{
  makeup_matrix();
}

void camera::makeup_matrix()
{
  view_mtx_ = matrix::look_at(eye_, at_, up_);
  projection_mtx_ = matrix::perspective(fov_, aspect_, z_near_, z_far_);
}


camera_control::camera_control(const camera& c)
  : initial_dir_(normalize(c.eye() - c.at())), center_(c.at()), radius_(len(c.eye() - c.at())),
    speed_{ 5.f, 5.f }, cur_rot_{}, rot_{}, button_(-1), prev_pos_{}
{
}

void camera_control::on_mouse_button(GLFWwindow *window, int button, int action, int mod)
{
  if (action == GLFW_PRESS) {
    button_ = button;
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
      double x, y;
      glfwGetCursorPos(window, &x, &y);
      int width, height;
      glfwGetWindowSize(window, &width, &height);
      prev_pos_ = vec2{ (float)(x * 2 - width) / width, (float)(height - 2 * y) / height };
      cur_rot_ = vec3::zero();
    }
  } else {
    button_ = -1;
    rot_ += cur_rot_;
    cur_rot_ = vec3::zero();
  }
}

void camera_control::on_cursor_move(GLFWwindow *window, double x, double y)
{
  switch (button_) {
  case GLFW_MOUSE_BUTTON_RIGHT:
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    vec2 cur_pos{ (float)(x * 2 - width) / width, (float)(height - 2 * y) / height };
    cur_rot_.x = (cur_pos.y - prev_pos_.y) * speed_.x;
    cur_rot_.y = (cur_pos.x - prev_pos_.x) * speed_.y;
    break;
  }
}

void camera_control::on_cursor_enter(GLFWwindow*, int enter)
{
  if (enter == GLFW_FALSE) {
    button_ = -1;
    rot_ += cur_rot_;
    cur_rot_ = vec3::zero();
  }
}

void camera_control::on_scroll(GLFWwindow*, double xoffset, double yoffset)
{
  radius_ += 1.f * float(yoffset);
  radius_ = std::max(radius_, 0.1f);
}

void camera_control::apply_to(camera *c)
{
  vec3 rot = rot_ + cur_rot_;
  vec3 dir = transform_direction(
    concat(matrix::rotate_x(rot.x), matrix::rotate_y(rot.y)),
//    matrix::rotate_y(rot.y),
    initial_dir_) *  radius_;

  c->set_at(center_);
  c->set_eye(center_ + dir);
  c->makeup_matrix();
}

