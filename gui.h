#pragma once


#include "font.h"
#include "shader.h"
#include "util.h"

namespace gui
{

typedef std::u32string string_t;
typedef string_t::value_type char_t;

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
  float tickness;
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

enum KeyAction
{
  KeyAction_Release,
  KeyAction_Press,
  KeyAction_Repeat,
};

enum Key
{
  Key_Enter,
  Key_Backspace,
  Key_Delete,
  Key_Left,
  Key_Right,
  Key_Home,
  Key_End,
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


inline void to_s_apply_opt(std::basic_stringstream<char_t>& ss) {}
template<class OptT, class... Opts>
void to_s_apply_opt(std::basic_stringstream<char_t>& ss, OptT&& opt, Opts&&... opts)
{
  ss << opt;
  to_s_apply_opt(ss, opts...);
}
template<class T, class... Opts>
string_t to_s(const T& v, Opts&&... opts)
{
  std::basic_stringstream<char_t> ss;
  to_s_apply_opt(ss, opts...);
  ss << v;
  return ss.str();
}
template<class T>
T from_s(const string_t& s)
{
  std::basic_stringstream<char_t> ss;
  ss << s;
  T v;
  ss >> v;
  return v;
}


struct draw_context;
struct calc_layout_context;

class component : public std::enable_shared_from_this<component>
{
public:
  typedef std::shared_ptr<component> ptr_t;
  typedef std::stack<std::pair<ptr_t, int>> event_handler_stack_t;

public:
  virtual bool update() { return false; };
  virtual void draw(draw_context&) const =0;
  virtual vec2 calc_layout(calc_layout_context&) { return local_pos(); }
  virtual bool make_event_handler_stack(const vec2& p, event_handler_stack_t&);

  virtual event_result on_mouse_button(const vec2&, MouseButton, MouseAction, ModKey);
  virtual event_result on_cursor_move(const vec2&);
  virtual event_result on_cursor_enter(const vec2&);
  virtual event_result on_cursor_leave(const vec2&);
  virtual event_result on_mouse_scroll(const vec2&);
  virtual event_result on_input_key(int key, int scancode, KeyAction action, ModKey mod);
  virtual event_result on_input_char(char_t code);
  virtual event_result on_acquire_focus();
  virtual event_result on_lost_focus();

  void set_local_pos(const vec2& p) { local_pos_ = p; }
  const vec2& local_pos() const { return local_pos_; }
  void set_size(const vec2& s) { size_ = s; }
  const vec2& size() const { return size_; }
  void set_name(const string_t& s) { name_ = s; }
  const string_t& name() const { return name_; }
  void set_layout_way(LayoutWay l) { layout_way_ = l; }
  LayoutWay layout_way() const { return layout_way_; }

protected:
  component(const string_t& name);

private:
  string_t name_;
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
  bool update() override;
  void draw(draw_context&) const override;
  vec2 calc_layout(calc_layout_context&) override;
  bool make_event_handler_stack(const vec2& p, event_handler_stack_t&) override;

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
  component_set(const string_t&name) : component(name) {}

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

  bool update();
  void draw();

  void set_screen_size(int w, int h) { screen_size_ = { (float)w, (float)h }; }
  void calc_layout();
  void recalc_layout();
  bool make_event_handler_stack(const vec2& p, event_handler_stack_t&) override;

  std::shared_ptr<window> add_window(const string_t& name);
  std::shared_ptr<window> add_window(std::shared_ptr<window>);
  void remove_window(component_ptr_t);
  void clear_window();

  bool on_mouse_button_root(vec2, MouseButton, MouseAction, ModKey);
  bool on_cursor_move_root(vec2);
  bool on_cursor_enter_root(vec2);
  bool on_cursor_leave_root(vec2);
  bool on_mouse_scroll_root(const vec2&);
  bool on_input_key_root(int key, int scancode, KeyAction action, ModKey mod);
  bool on_input_char_root(char_t code);

  void set_focus(component_ptr_t);
  component_ptr_t focus() const;

  font::renderer::ptr_t font_renderer() { return font_renderer_; }
  system_property& property() { return property_; }

protected:
  system(shader::ptr_t, font::renderer::ptr_t);

private:
  shader::ptr_t shader_;
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
  const GLuint vertex_buffer;
  const font::renderer::ptr_t font_renderer;
  const vec2 screen_size;
  const system_property& property;
  const component::ptr_t focused;

  void draw(int primitive_type,
            const vertex*, int vertex_cnt,
            const uint32_t *index_array, int index_cnt,
            const color& c0, const color& c1);
  void draw_triangle(const vec2& p0, const vec2& p1, const vec2& p2, const color& c);
  void draw_rect(const vec2& pos, const vec2& size, const color& c0, const color& c1);
  void draw_line(const vec2& a, const vec2& b, const color& c);
  void draw_font(const vec2& pos, const color& c, const string_t& str);

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

  event_result on_mouse_button(const vec2& base_pos, const vec2&, MouseButton, MouseAction, ModKey);
  std::pair<event_result, vec2> on_cursor_move(const vec2&);
  event_result on_lost_focus();

  vec2 base_pos() const { return base_pos_; }
  bool in_drag() const { return on_drag_; }

  void set_area(const vec2& pos, const vec2& size) { area_pos_ = pos; area_size_ = size; }

private:
  vec2 start_pos_;
  vec2 base_pos_;
  vec2 area_pos_;
  vec2 area_size_;
  bool on_drag_;
};


class textedit_control
{
public:
  textedit_control(size_t len = 20);
  virtual ~textedit_control() =default;

  void draw_font(const vec2& base_pos, draw_context& cxt) const;
  void draw_cursor(const vec2& base_pos, draw_context& cxt) const;

  event_result on_input_key(int key, int scancode, KeyAction action, ModKey mod);
  event_result on_input_char(char_t code);
  
  const string_t& str() const { return str_; }
  size_t cursor() const { return cursor_; }

  void set_str(const string_t& str) { str_ = str; }
  void set_cursor(size_t cursor) { cursor_ = cursor; }

private:
  string_t str_;
  size_t cursor_;
  size_t max_len_;
};


class window : public component_set, public shared_ptr_creator<window>
{
public:
  typedef std::shared_ptr<window> ptr_t;

public:
  window(const string_t& name, bool opened = true);

  void draw(draw_context&) const override;
  vec2 calc_layout(calc_layout_context&) override;
  bool make_event_handler_stack(const vec2& p, event_handler_stack_t&) override;

  event_result on_mouse_button(const vec2&, MouseButton, MouseAction, ModKey) override;
  event_result on_cursor_move(const vec2&) override;
  event_result on_cursor_enter(const vec2&) override;
  event_result on_cursor_leave(const vec2&) override;

private:
  vec2 name_pos_;
  vec2 tri_size_;
  vec2 line_l_;
  vec2 line_r_;
  drag_control drag_;

  bool in_open_;
  bool in_press_;
  bool in_over_;
};


class group : public component_set, public shared_ptr_creator<group>
{
public:
  typedef std::shared_ptr<group> ptr_t;

public:
  group(const string_t& name, bool opened = true);

  void draw(draw_context&) const override;
  vec2 calc_layout(calc_layout_context&) override;
  bool make_event_handler_stack(const vec2& p, event_handler_stack_t&) override;

  event_result on_mouse_button(const vec2&, MouseButton, MouseAction, ModKey) override;
  event_result on_cursor_enter(const vec2&) override;
  event_result on_cursor_leave(const vec2&) override;

protected:
  void draw_frame(draw_context&) const;

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
  button(const string_t& name, bool *notice);
  button(const string_t& name, callback_t);

  bool update() override;
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
  typedef std::vector<string_t> item_array_t;

public:
  combo_box(const string_t& name, int *value);

  void draw(draw_context&) const override;
  vec2 calc_layout(calc_layout_context&) override;

  event_result on_mouse_button(const vec2&, MouseButton, MouseAction, ModKey) override;
  event_result on_cursor_move(const vec2&) override;
  event_result on_cursor_enter(const vec2&) override;
  event_result on_cursor_leave(const vec2&) override;
  event_result on_lost_focus();

  int add_item(const string_t&);
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
  check_box(const string_t& name, bool *value);

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
  radio_button(const string_t& name, int *variable, int value);

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
  label(const string_t& name);

  void draw(draw_context&) const override;
  vec2 calc_layout(calc_layout_context&) override;

private:
  vec2 name_pos_;
};


class text_box : public component, public shared_ptr_creator<text_box>
{
public:
  typedef std::shared_ptr<text_box> ptr_t;
  typedef std::function<string_t ()> string_function_t;

  static const float DEFAULT_WIDTH;

public:
  text_box(const string_t& name, string_function_t, float width = DEFAULT_WIDTH);

  bool update() override;
  void draw(draw_context&) const override;
  vec2 calc_layout(calc_layout_context&) override;

private:
  vec2 name_pos_;
  vec2 text_size_;
  vec2 text_font_pos_;

  float width_;

  string_function_t string_function_;
  string_t str_;
};


class slider_base : public component
{
public:
  typedef std::shared_ptr<slider_base> ptr_t;

  static const float DEFAULT_WIDTH;

public:
  slider_base(const string_t& name, float width = DEFAULT_WIDTH);

  void draw(draw_context&) const override;
  vec2 calc_layout(calc_layout_context&) override;

  event_result on_mouse_button(const vec2&, MouseButton, MouseAction, ModKey) override;
  event_result on_cursor_move(const vec2&) override;
  event_result on_cursor_enter(const vec2&) override;
  event_result on_cursor_leave(const vec2&) override;
  event_result on_lost_focus();

protected:
  virtual void set_value(float width) =0;
  virtual string_t value_str() const =0;
  virtual float value_width(float width) const =0;

private:
  vec2 name_pos_;
  vec2 base_pos_;
  vec2 base_size_;
  vec2 value_size_;
  vec2 value_font_pos_;

  float width_;
  bool in_over_;
  drag_control drag_;
};


template<class T>
class slider : public slider_base, public shared_ptr_creator<slider<T>>
{
public:
  std::shared_ptr<slider> ptr_t;
  typedef T value_t;

public:
  slider(const string_t& name, value_t *val, value_t mn, value_t mx, float width = slider_base::DEFAULT_WIDTH)
    : slider_base(name, width), value_(val), min_value_(mn), max_value_(mx) {}

protected:
  void set_value(float width) override
  {
    *value_ = (value_t)((max_value_ - min_value_) * width + min_value_);
    *value_ = clamp(*value_, min_value_, max_value_);
  }
  string_t value_str() const override { return to_s(*value_); }
  float value_width(float width) const override
  {
    return (width * (*value_ - min_value_)) / (max_value_ - min_value_);
  }

private:
  value_t *value_;
  value_t min_value_;
  value_t max_value_;
};


class numeric_up_down_base : public component
{
public:
  typedef std::shared_ptr<numeric_up_down_base> ptr_t;

  static const float DEFAULT_WIDTH;

public:
  numeric_up_down_base(const string_t& name, float width = DEFAULT_WIDTH);

  void draw(draw_context&) const override;
  vec2 calc_layout(calc_layout_context&) override;
  bool make_event_handler_stack(const vec2& p, event_handler_stack_t&) override;

  event_result on_mouse_button(const vec2&, MouseButton, MouseAction, ModKey) override;
  event_result on_cursor_move(const vec2&) override;
  event_result on_cursor_enter(const vec2&) override;
  event_result on_cursor_leave(const vec2&) override;
  event_result on_input_key(int key, int scancode, KeyAction action, ModKey mod) override;
  event_result on_input_char(char_t code) override;
  event_result on_lost_focus() override;

protected:
  virtual void increment() =0;
  virtual void decrement() =0;
  virtual string_t str_value() const =0;
  virtual void value_str(const string_t& str) =0;
  virtual bool is_accept_char(char_t code) const { return true; }

  void set_str(const string_t& str) { textedit_.set_str(str); };

private:
  void end_textedit();

private:
  vec2 name_pos_;
  vec2 value_size_;
  vec2 value_font_pos_;
  vec2 up_pos_;
  vec2 up_size_;
  vec2 down_pos_;
  vec2 down_size_;

  textedit_control textedit_;
  
  float width_;
  bool in_over_up_;
  bool in_over_down_;
  bool in_over_textedit_;
  bool in_press_up_;
  bool in_press_down_;
  bool in_press_textedit_;
  bool in_textedit_;
};

template<class T>
class numeric_up_down : public numeric_up_down_base, public shared_ptr_creator<numeric_up_down<T>>
{
public:
  typedef T value_t;
  typedef std::function<void ()> callback_t;

public:
  numeric_up_down(const string_t& name,
                  value_t* value, value_t mn, value_t mx, value_t inc = value_t(1),
                  callback_t callback = 0,
                  float width = numeric_up_down_base::DEFAULT_WIDTH)
    : numeric_up_down_base(name, width),
      value_(value), min_value_(mn), max_value_(mx), value_inc_(inc), callback_(callback)
  {
    set_str(str_value());
  }

protected:
  void increment() override { change_value(*value_ + value_inc_); }
  void decrement() override { change_value(*value_ - value_inc_); }
  string_t str_value() const override { return to_s(*value_); }
  void value_str(const string_t& str) override { change_value(from_s<T>(str)); }
  bool is_accept_char(char_t code) const override
  {
    if constexpr (std::is_integral<T>::value) {
      if constexpr (std::is_signed<T>::value) {
        return ((code >= U'0') && (code <= U'9')) || (code == U'-') || (code == U'+');
      } else {
        return ((code >= U'0') && (code <= U'9'));
      }
    } else if constexpr (std::is_floating_point<T>::value) {
        return ((code >= U'0') && (code <= U'9')) || (code == U'-') || (code == U'+') || (code == '.');
    } else {
      return true;
    }
  }

private:
  void change_value(value_t value)
  {
    value = clamp(value, min_value_, max_value_);
    if (*value_ != value) {
      *value_ = value;
      set_str(str_value());
      callback_();
    }
  }
  
private:
  value_t *value_;
  value_t min_value_;
  value_t max_value_;
  value_t value_inc_;
  callback_t callback_;
};


} // end of namepsace gui

