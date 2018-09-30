
#pragma once

class camera
{
public:
  camera();
  camera(const vec3& eye,
         const vec3& at,
         const vec3& up,
         float fov,
         float aspect,
         float z_near,
         float z_far);
  
  const vec3& eye() const { return eye_; }
  const vec3& at() const { return at_; }
  const vec3& up() const { return up_; }
  const matrix& view_matrix() const { return view_mtx_; }
  float fov() const { return fov_; }
  float aspect() const { return aspect_; }
  float z_near() const { return z_near_; }
  float z_far() const { return z_far_; }
  const matrix& projection_matrix() const { return projection_mtx_; }

  void set_eye(const vec3& eye) { eye_ = eye; }
  void set_at(const vec3& at) { at_ = at; }
  void set_up(const vec3& up) { up_ = up; }
  void set_fov(float fov) { fov_ = fov; }
  void set_aspect(float aspect) { aspect_ = aspect; }
  void set_z_near(float z_near) { z_near_ = z_near; }
  void set_z_far(float z_far) { z_far_ = z_far; }

  void makeup_matrix();

private:
  vec3 eye_;
  vec3 at_;
  vec3 up_;

  float fov_;
  float aspect_;
  float z_near_;
  float z_far_;

  matrix view_mtx_;
  matrix projection_mtx_;
};


class camera_control
{
public:
  typedef std::shared_ptr<camera_control> ptr_t;

public:
  camera_control(const camera&);

  void on_mouse_button(GLFWwindow*, int, int, int);
  void on_cursor_move(GLFWwindow*, double, double);
  void on_cursor_enter(GLFWwindow*, int);
  void on_scroll(GLFWwindow*, double, double);

  void apply_to(camera*);

  void set_speed(const vec2& v) { speed_ = v;}
  const vec2& speed() const { return speed_; }

private:
  vec3 initial_dir_;
  vec3 center_;
  float radius_;
  vec2 speed_;

  vec3 cur_rot_;
  vec3 rot_;

  int button_;
  vec2 prev_pos_;
};
