#pragma once


#include "font.h"
#include "shader.h"
#include "util.h"

namespace gui
{

typedef std::u16string string;

struct vertex
{
  vec2 pos, uv;
};

enum LayoutWay {
  LayoutWay_Vertical,
  LayoutWay_Horizon,
};


struct system_property
{
  color font_color;
  int font_size;
  color frame_color0;
  color frame_color1;
  color active_color;
  color semiactive_color;

  float round;
  float mergin;
};

enum MouseButton
{
  MouseButton_Left,
  MouseButton_Right,
  MouseButton_Middle,
};

enum MouseAction {
  MouseAction_Release,
  MouseAction_Press,
};

enum ModKey
{
  ModKey_None = 0x00,
  ModKey_Shift = 0x01,
  ModKey_Control = 0x02,
  ModKey_Alt = 0x04,

  ModKey_All = ModKey_Shift | ModKey_Control | ModKey_Alt,
};

enum KeyAction {
  KeyAction_Release,
  KeyAction_Press,
  KeyAction_Repeat,
};

struct event_result
{
  bool accept;
  bool recalc_layout;

  event_result()
    : event_result(false, false) {}
  event_result(bool accept, bool recalc)
    : accept(accept), recalc_layout(recalc) {}
};


struct draw_context;
struct calc_layout_context;

class component : public std::enable_shared_from_this<component>
{
public:
  typedef std::shared_ptr<component> ptr_t;
  typedef std::stack<ptr_t> event_handler_stack_t;

public:
  virtual void update() {};
  virtual void draw(draw_context&) const =0;
  virtual vec2 calc_layout(calc_layout_context&) { return local_pos(); }
  virtual void make_event_handler_stack(const vec2& p, event_handler_stack_t&);

  virtual event_result on_mouse_button(const vec2&, MouseButton, MouseAction, ModKey);
  virtual event_result on_cursor_move(const vec2&);
  virtual event_result on_cursor_enter(const vec2&);
  virtual event_result on_cursor_leave(const vec2&);
  virtual event_result on_mouse_scroll(const vec2&);
  virtual event_result on_input_key(int key, int scancode, KeyAction action, ModKey mod);
  virtual event_result on_input_char(char16_t code);
  virtual event_result on_acquire_focus();
  virtual event_result on_lost_focus();

  void set_local_pos(const vec2& p) { local_pos_ = p; }
  const vec2& local_pos() const { return local_pos_; }
  void set_size(const vec2& s) { size_ = s; }
  const vec2& size() const { return size_; }
  void set_name(const string& s) { name_ = s; }
  const string& name() const { return name_; }
  void set_layout_way(LayoutWay l) { layout_way_ = l; }
  LayoutWay layout_way() const { return layout_way_; }

protected:
  component(const string& name);

private:
  string name_;
  vec2 local_pos_;
  vec2 size_;
  LayoutWay layout_way_;
};

class component_set : public component
{
public:
  typedef component::ptr_t component_ptr_t;
  typedef std::vector<component_ptr_t> array_t;

public:
  void update() override;
  void draw(draw_context&) const override;
  vec2 calc_layout(calc_layout_context&) override;
  void make_event_handler_stack(const vec2& p, event_handler_stack_t&) override;

  template<class T, class... Args>
  typename T::ptr_t add_child(Args... args)
  {
    auto p = std::make_shared<T>(args...);
    child_array_.push_back(p);
    return p;
  }
  void add_child(component_ptr_t);
  void remove_child(component_ptr_t);
  void clear_child();

protected:
  array_t& child_array() { return child_array_; }

protected:
  component_set(const string&name) : component(name) {}

private:
  array_t child_array_;

};

class window;

class system : protected component_set, public shared_ptr_creator<system>
{
public:
  using shared_ptr_creator::ptr_t;

public:
  ~system();

  void update();
  void draw();

  void set_screen_size(int w, int h) { screen_size_ = { (float)w, (float)h }; }
  void calc_layout();
  void make_event_handler_stack(const vec2& p, event_handler_stack_t&) override;

  std::shared_ptr<window> add_window(const string& name);
  std::shared_ptr<window> add_window(std::shared_ptr<window>);
  void remove_window(component_ptr_t);
  void clear_window();

  bool on_mouse_button_root(vec2, MouseButton, MouseAction, ModKey);
  bool on_cursor_move_root(vec2);
  bool on_cursor_enter_root(vec2);
  bool on_cursor_leave_root(vec2);
  bool on_mouse_scroll_root(const vec2&);
  bool on_input_key_root(int key, int scancode, KeyAction action, ModKey mod);
  bool on_input_char_root(char16_t code);

  void set_focus(component_ptr_t);
  component_ptr_t focus() const;

  font::renderer::ptr_t font_renderer() { return font_renderer_; }
  system_property& property() { return property_; }

protected:
  system(shader::ptr_t, texture::ptr_t, font::renderer::ptr_t);

  void recalc_layout();

private:
  shader::ptr_t shader_;
  texture::ptr_t texture_;
  font::renderer::ptr_t font_renderer_;
  GLuint vertex_buffer_;
  vec2 screen_size_;

  system_property property_;

  vec2 prev_cursor_pos_;
  std::weak_ptr<component> focused_;
};

struct draw_context
{
  const shader::ptr_t gui_shader;
  const texture::ptr_t gui_tex;
  const GLuint vertex_buffer;
  const font::renderer::ptr_t font_renderer;
  const vec2 screen_size;
  const system_property& property;
  const component::ptr_t focused;

  void draw(const vertex*, int vertex_cnt, const uint32_t *index_array, int index_cnt, const color& c0, const color& c1);
  void draw_triangle(const vec2& p0, const vec2& p1, const vec2& p2, const color& c);
  void draw_rect(const vec2& pos, const vec2& size, const color& c0, const color& c1);
  void draw_font(const vec2& pos, const color& c, const string& str);

  bool is_focused(component::ptr_t p) { return focused == p; }
};

struct calc_layout_context
{
  const font::renderer::ptr_t font_renderer;
  const vec2 screen_size;
  const system_property& property;
};


class drag_control
{
public:
  drag_control();
  virtual ~drag_control() =default;

  event_result on_mouse_button(component::ptr_t, const vec2&, MouseButton, MouseAction, ModKey);
  event_result on_cursor_move(component::ptr_t, const vec2&);

  void set_area(const vec2& pos, const vec2& size) { area_pos_ = pos; area_size_ = size; }

private:
  vec2 start_pos_;
  vec2 cur_pos_;
  vec2 area_pos_;
  vec2 area_size_;
  bool on_drag_;
};


class window : public component_set, public shared_ptr_creator<window>
{
public:
  typedef std::shared_ptr<window> ptr_t;

public:
  window(const string& name);

  void draw(draw_context&) const override;
  vec2 calc_layout(calc_layout_context&) override;

  event_result on_mouse_button(const vec2&, MouseButton, MouseAction, ModKey) override;
  event_result on_cursor_move(const vec2&) override;

private:
  vec2 name_pos_;
  drag_control drag_;
};


class group : public component_set, public shared_ptr_creator<group>
{
public:
  typedef std::shared_ptr<group> ptr_t;

public:
  group(const string& name, bool opened = true);

  void draw(draw_context&) const override;
  vec2 calc_layout(calc_layout_context&) override;
  void make_event_handler_stack(const vec2& p, event_handler_stack_t&) override;

  event_result on_mouse_button(const vec2&, MouseButton, MouseAction, ModKey) override;
  event_result on_cursor_enter(const vec2&) override;
  event_result on_cursor_leave(const vec2&) override;

private:
  vec2 name_pos_;
  vec2 tri_size_;
  vec2 line_pos_;
  vec2 line_size_;
  vec2 line_lt_;

  bool in_open_;
  bool in_press_;
  bool in_over_;
};


class button : public component, public shared_ptr_creator<button>
{
public:
  typedef std::shared_ptr<button> ptr_t;
  typedef std::function<void ()> callback_t;

public:
  button(const string& name, bool *notice);
  button(const string& name, callback_t);

  void update() override;
  void draw(draw_context&) const override;
  vec2 calc_layout(calc_layout_context&) override;

  event_result on_mouse_button(const vec2&, MouseButton, MouseAction, ModKey) override;
  event_result on_cursor_enter(const vec2&) override;
  event_result on_cursor_leave(const vec2&) override;

private:
  vec2 name_pos_;
  vec2 area_size_;

  bool in_press_;
  bool in_over_;
  bool *notice_variable_;
  callback_t notice_function_;
};


class combo_box : public component, public shared_ptr_creator<combo_box>
{
public:
  typedef std::shared_ptr<combo_box> ptr_t;
  typedef std::vector<string> item_array_t;

public:
  combo_box(const string& name, int *value);

  void draw(draw_context&) const override;
  vec2 calc_layout(calc_layout_context&) override;

  event_result on_mouse_button(const vec2&, MouseButton, MouseAction, ModKey) override;
  event_result on_cursor_move(const vec2&) override;
  event_result on_cursor_enter(const vec2&) override;
  event_result on_cursor_leave(const vec2&) override;
  event_result on_lost_focus();

  int add_item(const string&);
  void remove_item(int);
  void clear_item();

protected:
  int get_item_list_index(const vec2&);

private:
  vec2 name_pos_;
  vec2 item_pos_;
  vec2 item_size_;
  vec2 item_font_pos_;
  vec2 tri_pos_;
  vec2 tri_size_;
  vec2 item_list_pos_;
  vec2 item_list_size_;
  vec2 item_list_font_pos_;
  float item_height_;

  item_array_t item_array_;

  bool in_open_;
  bool in_press_;
  bool in_over_;
  int cur_index_;
  int *notice_variable_;
};


class check_box : public component, public shared_ptr_creator<check_box>
{
public:
  typedef std::shared_ptr<check_box> ptr_t;

public:
  check_box(const string& name, bool *value);

  void draw(draw_context&) const override;
  vec2 calc_layout(calc_layout_context&) override;

  event_result on_mouse_button(const vec2&, MouseButton, MouseAction, ModKey) override;
  event_result on_cursor_enter(const vec2&) override;
  event_result on_cursor_leave(const vec2&) override;

private:
  vec2 name_pos_;
  vec2 box_pos_;
  vec2 box_size_;

  bool in_press_;
  bool in_over_;
  bool *notice_variable_;
};


class radio_button : public component, public shared_ptr_creator<radio_button>
{
public:
  typedef std::shared_ptr<radio_button> ptr_t;

public:
  radio_button(const string& name, int *variable, int value);

  void draw(draw_context&) const override;
  vec2 calc_layout(calc_layout_context&) override;

  event_result on_mouse_button(const vec2&, MouseButton, MouseAction, ModKey) override;
  event_result on_cursor_enter(const vec2&) override;
  event_result on_cursor_leave(const vec2&) override;

private:
  vec2 name_pos_;
  vec2 box_pos_;
  vec2 box_size_;

  bool in_press_;
  bool in_over_;
  int *notice_variable_;
  int value_;
};


class label : public component, public shared_ptr_creator<label>
{
public:
  typedef std::shared_ptr<label> ptr_t;

public:
  label(const string& name);

  void draw(draw_context&) const override;
  vec2 calc_layout(calc_layout_context&) override;

private:
  vec2 name_pos_;
};

} // end of namepsace gui

