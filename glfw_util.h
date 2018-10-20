
#pragma once

#include "gui.h"


namespace gui
{

inline vec2 pos_from_glfw(GLFWwindow *window, double x, double y)
{
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  return { (float)x, (float)y };
}

inline vec2 get_current_cursor_pos(GLFWwindow *window)
{
  double x, y;
  glfwGetCursorPos(window, &x, &y);
  return pos_from_glfw(window, x, y);
}

inline bool glfw_mouse_button(system::ptr_t sys, GLFWwindow *window, int button, int action, int mod)
{
  return sys->on_mouse_button_root(get_current_cursor_pos(window), (MouseButton)button, (MouseAction)action, (ModKey)mod);
}

inline bool glfw_cursor_move(system::ptr_t sys, GLFWwindow *window, double x, double y)
{
  return sys->on_cursor_move_root(pos_from_glfw(window, x, y));
}

inline bool glfw_cursor_enter(system::ptr_t sys, GLFWwindow *window, int entered)
{
  if (entered) {
    return sys->on_cursor_enter_root(get_current_cursor_pos(window));
  } else {
    return sys->on_cursor_leave_root(get_current_cursor_pos(window));
  }
}

inline bool glfw_mouse_scroll(system::ptr_t sys, GLFWwindow *, double x, double y)
{
  return sys->on_mouse_scroll_root({(float)x, (float)y});
}

inline bool glfw_input_key(system::ptr_t sys, GLFWwindow *, int key, int scancode, int action, int mod)
{
  switch (key) {
#define DEF_KEY(glfw, gui) case GLFW_KEY_##glfw: key = Key_##gui; break
DEF_KEY(ENTER, Enter);
DEF_KEY(BACKSPACE, Backspace);
DEF_KEY(DELETE, Delete);
DEF_KEY(LEFT, Left);
DEF_KEY(RIGHT, Right);
DEF_KEY(HOME, Home);
DEF_KEY(END, End);
#undef DEF_KEY
  }

  return sys->on_input_key_root(key, scancode, (KeyAction)action, (ModKey)mod);
}
 
inline bool glfw_input_char(system::ptr_t sys, GLFWwindow *, unsigned int code)
{
  return sys->on_input_char_root(code);
}


} // end of namespace gui
