
#include "stdafx.h"

#include "gui.h"
#include "util.h"

namespace gui
{

namespace {

vertex_decl gui_vertex_decl[] = {
  { Semantics_Position, GL_FLOAT, 2, offsetof(gui::vertex, pos), sizeof(gui::vertex) },
  { Semantics_TexCoord_0, GL_FLOAT, 2, offsetof(gui::vertex, uv), sizeof(gui::vertex) },
};


bool is_in_area(const vec2& p, const vec2& pos, const vec2& size)
{
  if (p.x < pos.x) { return false; }
  if (p.y < pos.y) { return false; }
  if (p.x > (pos.x + size.x)) { return false; }
  if (p.y > (pos.y + size.y)) { return false; }

  return true;
}

vec2 place_compoent_array(vec2 pos, calc_layout_context& cxt, std::vector<component::ptr_t>& component_array)
{
  const system_property& prop = cxt.property;
  const auto orig = pos;
  const float bol = orig.x;
  float width = 0.f;
  for (auto c : component_array) {
    c->set_local_pos(pos);
    auto diff = c->calc_layout(cxt);
    width = std::max(width, pos.x + diff.x - orig.x);
    switch (c->layout_way()) {
    case LayoutWay_Vertical:
      pos.x = bol;
      pos.y += diff.y + prop.mergin;
      break;
    case LayoutWay_Horizon:
      pos.x += diff.x + prop.mergin;
      break;
    }
  }
  return { width, pos.y - orig.y };
}

} // end of namespace



void draw_context::draw(int primitive_type,
                        const vertex* vertex_array, int vertex_cnt,
                        const uint32_t *index_array, int index_cnt,
                        const color& c0, const color& c1)
{
  gui_shader->use();

  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, vertex_cnt * sizeof(vertex), vertex_array, GL_STATIC_DRAW);

  for (const auto& decl: gui_vertex_decl) {
    gui_shader->set_attrib(decl);
  }
  gui_shader->set_uniform("screen_size", screen_size);
  gui_shader->set_uniform("color0", c0);
  gui_shader->set_uniform("color1", c1);
  gui_shader->set_uniform("tickness", clamp(property.tickness / property.round / 2.f, 0.f, 0.5f));

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  glDrawElements(primitive_type,
                 index_cnt,
                 GL_UNSIGNED_INT,
                 index_array);
}

void draw_context::draw_triangle(const vec2& p0, const vec2& p1, const vec2& p2, const color& col)
{
  vec2 c = (p0 + p1 + p2) / 3.f;
  const vertex vertex_array[] = {
    { c,  { 0.f, 0.f} },
    { p0, { 1.f, 1.f} },
    { p1, { 1.f, 1.f} },
    { p2, { 1.f, 1.f} },
  };
  static const uint32_t index_array[] = {
    0, 1, 2, 0, 2, 3, 0, 3, 1
  };
  draw(GL_TRIANGLES, vertex_array, countof(vertex_array), index_array, countof(index_array), col, col);
}

void draw_context::draw_rect(const vec2& pos, const vec2& size, const color& c0, const color& c1)
{
  auto [x, y] = pos;
  auto [w, h] = size;
  float r = property.round;
  r = std::min(r, std::min(w, h) / 2.f);
  const vertex vertex_array[] = {
    // TL
    { { x,     y     }, { .0f, .0f} },
    { { x,     y + r }, { .0f, .5f} },
    { { x + r, y     }, { .5f, .0f} },
    { { x + r, y + r }, { .5f, .5f} },
    // TR
    { { x + w - r, y     }, { .5f, .0f} },
    { { x + w - r, y + r }, { .5f, .5f} },
    { { x + w,     y     }, { .0f, .0f} },
    { { x + w,     y + r }, { .0f, .5f} },
    // BL
    { { x,     y + h - r }, { .0f, .5f} },
    { { x,     y + h     }, { .0f, .0f} },
    { { x + r, y + h - r }, { .5f, .5f} },
    { { x + r, y + h     }, { .5f, .0f} },
    // BR
    { { x + w - r, y + h - r }, { .5f, .5f} },
    { { x + w - r, y + h     }, { .5f, .0f} },
    { { x + w,     y + h - r }, { .0f, .5f} },
    { { x + w,     y + h     }, { .0f, .0f} },
  };
  static const uint32_t index_array[] = {
    0, 1, 2, 2, 1, 3, 2, 3, 4, 4, 3, 5, 4, 5, 6, 6, 5, 7,
    1, 8, 3, 3, 8, 10, 3, 10, 5, 5, 10, 12, 5, 12, 7, 7, 12, 14,
    8, 9, 10, 10, 9, 11, 10, 11, 12, 12, 11, 13, 12, 13, 14, 14, 13, 15
  };
  draw(GL_TRIANGLES, vertex_array, countof(vertex_array), index_array, countof(index_array), c0, c1);
}

void draw_context::draw_line(const vec2& a, const vec2& b, const color& c)
{
  const vertex vertex_array[] = {
    { a, { .5f, .5f } },
    { b, { .5f, .5f } },
  };
  static const uint32_t index_array[] = {
    0, 1,
  };
  draw(GL_LINES, vertex_array, countof(vertex_array), index_array, countof(index_array), c, c);
}

void draw_context::draw_font(const vec2& pos, const color& c, const string_t& str)
{
  font_renderer->render(pos, { property.font_size, property.font_size }, c, str);
}



component::component(const string_t& name)
  : name_(name), local_pos_{}, size_{}, layout_way_(LayoutWay_Vertical)
{
}

bool component::make_event_handler_stack(const vec2& p, event_handler_stack_t& stk)
{
  if (is_in_area(p, local_pos(), size())) {
    stk.push({ shared_from_this(), 0 });
    return true;
  }
  return false;
}

event_result component::on_mouse_button(const vec2&, MouseButton, MouseAction, ModKey)
{
  return { false, false };
}

event_result component::on_cursor_move(const vec2&)
{
  return { false, false };
}

event_result component::on_cursor_enter(const vec2&)
{
  return { false, false };
}

event_result component::on_cursor_leave(const vec2&)
{
  return { false, false };
}

event_result component::on_mouse_scroll(const vec2&)
{
  return { false, false };
}

event_result component::on_input_key(int key, int scancode, KeyAction action, ModKey mod)
{
  return { false, false };
}

event_result component::on_input_char(char_t code)
{
  return { false, false };
}

event_result component::on_acquire_focus()
{
  return { false, false };
}

event_result component::on_lost_focus()
{
  return { false, false };
}


bool component_set::update()
{
  bool r = false;
  for (const auto& c : child_array_) {
    r = c->update() || r;
  }
  return r;
}

void component_set::draw(draw_context& cxt) const
{
  component::ptr_t focused;
  for (const auto& c : child_array_) {
    if (cxt.is_focused(c)) {
      focused = c;
      continue;
    }
    c->draw(cxt);
  }
}

vec2 component_set::calc_layout(calc_layout_context& cxt)
{
  set_size(place_compoent_array(local_pos(), cxt, child_array()));
  return size();
}

bool component_set::make_event_handler_stack(const vec2& p, event_handler_stack_t& stk)
{
  bool ret = component::make_event_handler_stack(p, stk);
  for (auto c : child_array()) {
    ret = c->make_event_handler_stack(p, stk) || ret;
  }
  return ret;
}

void component_set::add_child(component_ptr_t p) {
  child_array_.push_back(p);
}

void component_set::remove_child(component_ptr_t p) {
  child_array_.erase(std::find(
    child_array_.begin(), child_array_.end(), p));
}

void component_set::clear_child()
{
  child_array_.clear();
}



system::system(shader::ptr_t s, font::renderer::ptr_t f)
  : component_set(U"root"),
    shader_(s), font_renderer_(f),
    prev_cursor_pos_{}
{
  glGenBuffers(1, &vertex_buffer_);

  // デフォルトプロパティ.
  property_.font_color = color{ 0.f, 0.f, 0.f, 1.f };
  property_.font_size = 12;
  property_.frame_color0 = color{ 0.f, 0.f, 0.f, 1.f };
  property_.frame_color1 = color{ 1.f, 1.f, 1.f, .9f };
  property_.active_color = color{ 1.f, 0.65f, 0.0f, 1.0f };
  property_.semiactive_color = color{ 0.99f, 0.96f, 0.75f, 1.f };
  property_.round = 4.f;
  property_.mergin = 4.f;
  property_.tickness = 4.f;
}

system::~system()
{
  glDeleteBuffers(1, &vertex_buffer_);
}

bool system::update()
{
  bool r = component_set::update();
  if (r) {
    recalc_layout();
  }
  return true;
}

void system::draw()
{
  glDisable(GL_DEPTH_TEST);

  font_renderer_->set_screen_size((int)screen_size_.x, (int)screen_size_.y);

  draw_context cxt = {
    shader_, vertex_buffer_, font_renderer_, screen_size_, property_, focused_.lock() };

  component_set::draw(cxt);

  if (auto focused = focus()) {
    focused->draw(cxt);
  }
}

void system::calc_layout()
{
  calc_layout_context cxt = {
    font_renderer_, screen_size_, property_ };
  component_set::calc_layout(cxt);
}

void system::recalc_layout()
{
  calc_layout_context cxt = {
    font_renderer_, screen_size_, property_ };
  for (auto c : child_array()) {
    c->calc_layout(cxt);
  }
}

bool system::make_event_handler_stack(const vec2& p, event_handler_stack_t& stk)
{
  bool ret = false;
  for (auto c : reverse(child_array())) {
    ret = c->make_event_handler_stack(p, stk);
    if (ret) {
      break;
    }
  }
  return ret;
}

std::shared_ptr<window> system::add_window(const string_t& name)
{
  return add_child<window>(name);
}

std::shared_ptr<window> system::add_window(std::shared_ptr<window> p)
{
  add_child(p);
  return p;
}

void system::remove_window(component_ptr_t p)
{
  remove_child(p);
}

void system::clear_window()
{
  clear_child();
}

bool system::on_mouse_button_root(vec2 p, MouseButton button, MouseAction action, ModKey mod)
{
  p = clamp(p, vec2{}, screen_size_);
  // ウィンドウフォーカス.
  for (auto w : reverse(child_array())) {
    if (is_in_area(p, w->local_pos(), w->size())) {
      remove_child(w);
      add_child(w);	// 末尾へ回す.
      break;
    }
  }

  event_handler_stack_t stk;
  make_event_handler_stack(p, stk);
  event_result r;
  while (!stk.empty()) {
    r = stk.top().first->on_mouse_button(p, button, action, mod);
    if (r.accept) {
      set_focus(stk.top().first);
      break;
    }
    stk.pop();
  }
  if (!r.accept) {
    set_focus(0);
  }

  if (r.recalc_layout) {
    recalc_layout();
  }
  return r.accept;
}

bool system::on_cursor_move_root(vec2 p)
{
  p = clamp(p, vec2{}, screen_size_);
  event_handler_stack_t cur_stk, pre_stk;
  make_event_handler_stack(p, cur_stk);
  make_event_handler_stack(prev_cursor_pos_, pre_stk);

  // enter
  event_result r_enter;
  for (auto [c, index] : cur_stk) {
    if (std::find(begin(pre_stk), end(pre_stk), std::make_pair(c, index)) == end(pre_stk)) {
      r_enter = c->on_cursor_enter(p);
    }
  }
  // leave
  event_result r_leave;
  for (auto [c, index] : pre_stk) {
    if (std::find(begin(cur_stk), end(cur_stk), std::make_pair(c, index)) == end(cur_stk)) {
      r_leave = c->on_cursor_leave(p);
    }
  }
  // move
  event_result r_move;
  if (auto c = focus()) {
    r_move = c->on_cursor_move(p);
  }
  for (auto w : child_array()) {
    w->on_cursor_move(p);
  }

  if (r_enter.recalc_layout || r_leave.recalc_layout || r_move.recalc_layout) {
    recalc_layout();
  }
  prev_cursor_pos_ = p;

  return r_enter.accept || r_leave.accept || r_move.accept;
}

bool system::on_cursor_enter_root(vec2)
{
  return false;
}

bool system::on_cursor_leave_root(vec2 p)
{
  p = clamp(p, vec2{}, screen_size_);
  return 
    on_cursor_move_root(p) ||
      on_mouse_button_root(p, MouseButton_Left, MouseAction_Release, ModKey_All) ||
      on_mouse_button_root(p, MouseButton_Right, MouseAction_Release, ModKey_All) ||
      on_mouse_button_root(p, MouseButton_Middle, MouseAction_Release, ModKey_All);
}

bool system::on_mouse_scroll_root(const vec2& s)
{
  event_result r;
  if (auto c = focus()) {
    r = c->on_mouse_scroll(s);
  }
  if (r.recalc_layout) {
    recalc_layout();
  }
  return r.accept;
}

bool system::on_input_key_root(int key, int scancode, KeyAction action, ModKey mod)
{
  event_result r;
  if (auto c = focus()) {
    r = c->on_input_key(key, scancode, action, mod);
  }
  if (r.recalc_layout) {
    recalc_layout();
  }
  return r.accept;
}

bool system::on_input_char_root(char_t code)
{
  event_result r;
  if (auto c = focus()) {
    r = c->on_input_char(code);
  }
  if (r.recalc_layout) {
    recalc_layout();
  }
  return r.accept;
}


void system::set_focus(component_ptr_t p)
{
  auto prev = focused_.lock();
  if (p == prev) {
    return;
  }
  event_result r_lost;
  if (prev) {
    r_lost = prev->on_lost_focus();
  }
  event_result r_acquire;
  if (p) {
    r_acquire = p->on_acquire_focus();
  }
  if (r_lost.recalc_layout || r_acquire.recalc_layout) {
    recalc_layout();
  }
  focused_ = p;
}

system::component_ptr_t system::focus() const
{
  return focused_.lock();
}


drag_control::drag_control()
  : area_pos_{}, area_size_{}, on_drag_(false)
{
}

event_result drag_control::on_mouse_button(const vec2& base_pos, const vec2& p, MouseButton button, MouseAction action, ModKey)
{
  if ((button == MouseButton_Left)) {
    if (action == MouseAction_Press) {
      if (is_in_area(p, area_pos_, area_size_)) {
        base_pos_ = base_pos;
        start_pos_ = p;
        on_drag_ = true;
        return { true, false };
      }
    } else {
      if (on_drag_) {
        on_drag_ = false;
        return { true, false };
      }
    }
  }
  return { false, false };
}

std::pair<event_result, vec2> drag_control::on_cursor_move(const vec2& p)
{
  if (on_drag_) {
    vec2 diff = p - start_pos_;
    return { { true, false }, diff};
  }
  return { { false, false }, vec2::zero() };
}

event_result drag_control::on_lost_focus()
{
  on_drag_ = false;
  return { false, false };
}



textedit_control::textedit_control(size_t len)
  : cursor_(0), max_len_(len)
{
}

void textedit_control::draw_font(const vec2& base_pos, draw_context& cxt) const
{
  cxt.draw_font(base_pos, cxt.property.font_color, str_);
}

void textedit_control::draw_cursor(const vec2& base_pos, draw_context& cxt) const
{
  const system_property& prop = cxt.property;
  rect area = cxt.font_renderer->get_area(prop.font_size, {str_.c_str(), cursor_});
  cxt.draw_line(vec2{ base_pos.x + area.w, base_pos.y },
                vec2{ base_pos.x + area.w, base_pos.y - prop.font_size },
                cxt.property.frame_color0);
}

event_result textedit_control::on_input_key(int key, int scancode, KeyAction action, ModKey mod)
{
  if ((action == KeyAction_Press) || action == KeyAction_Repeat) {
    switch (key) {
    case Key_Enter:
      return { true, false };
    case Key_Backspace:
      if (cursor_ > 0) {
        --cursor_;
        str_.erase(str_.begin() + cursor_);
      }
      return { false, true };
    case Key_Delete:
      if (cursor_ < str_.size()) {
        str_.erase(str_.begin() + cursor_);
      }
      return { false, true };
    case Key_Left:
      if (cursor_ > 0) {
        --cursor_;
      }
      break;
    case Key_Right:
      if (cursor_ < str_.size()) {
        ++cursor_;
      }
      break;
    case Key_Home:
      cursor_ = 0;
      break;
    case Key_End:
      cursor_ = str_.size();
      break;
    }
  }
  return { false, false };
}

event_result textedit_control::on_input_char(char_t code)
{
  if (str_.length() >= max_len_) {
    return { false, false };
  }
  str_.insert(cursor_, 1, code);
  ++cursor_;
  return { true, true };
}



window::window(const string_t& name, bool opened )
  : component_set(name), in_open_(opened), in_press_(false), in_over_(false)
{
}

void window::draw(draw_context& cxt) const
{
  const system_property& prop = cxt.property;
  cxt.draw_rect(local_pos(), size(), prop.frame_color0, prop.frame_color1);
  cxt.draw_font(name_pos_, prop.font_color, name());

  if (in_over_) {
    cxt.draw_rect(local_pos(), tri_size_, prop.active_color, prop.semiactive_color);
  }
  const color& cc =  in_press_ ? prop.active_color : prop.frame_color0;
  auto [x, y] = local_pos();
  if (in_open_) {
    cxt.draw_triangle(vec2{ x + prop.mergin,               y + tri_size_.y * 3.f / 8.f },
                      vec2{ x + tri_size_.x * .5f,         y + tri_size_.y * 5.f / 8.f },
                      vec2{ x + tri_size_.x - prop.mergin, y + tri_size_.y * 3.f / 8.f },
                      cc);
    cxt.draw_line(line_l_, line_r_, prop.frame_color0);
    component_set::draw(cxt);
  } else {
    cxt.draw_triangle(vec2{ x + tri_size_.x / 4.f + prop.mergin, y + tri_size_.y / 4.f },
                      vec2{ x + tri_size_.x / 4.f + prop.mergin, y + tri_size_.y * 3.f / 4.f },
                      vec2{ x + tri_size_.x * 3.f / 4.f,         y + tri_size_.y / 2.f },
                      cc);
  }
}

vec2 window::calc_layout(calc_layout_context& cxt)
{
  const system_property& prop = cxt.property;
  tri_size_ = vec2{ prop.font_size / 2.f + prop.mergin * 3.f, prop.font_size + prop.mergin * 2.f };
  rect name_area = cxt.font_renderer->get_area(prop.font_size, name());
  float width = name_area.w + prop.mergin * 2.f;
  auto pos = local_pos();
  name_pos_ = pos + vec2{ prop.mergin + tri_size_.x, prop.font_size + prop.mergin };
  pos += vec2{ prop.mergin, prop.mergin * 2.f + prop.font_size };
  if (in_open_) {
    pos.y += prop.mergin * 2.f;
  }
  vec2 size = place_compoent_array(pos, cxt, child_array());
  if (!in_open_) {
    size.y = 0.f;
  }
  size.x = std::max(width, size.x + prop.mergin * 2.f);
  size.y = (pos.y + size.y) - local_pos().y;
  set_size(size);

  line_l_ = vec2{ pos.x, local_pos().y + tri_size_.y + prop.mergin };
  line_r_ = line_l_ + vec2{ size.x - prop.mergin * 2.f, 0.f };

  drag_.set_area(local_pos(), size);

  return size;
}

bool window::make_event_handler_stack(const vec2& p, event_handler_stack_t& stk)
{
  bool ret = component::make_event_handler_stack(p, stk);
  if (in_open_) {
    for (auto c : child_array()) {
      ret = c->make_event_handler_stack(p, stk) || ret;
    }
  }
  return ret;
}

event_result window::on_mouse_button(const vec2& p, MouseButton button, MouseAction action, ModKey mod)
{
  // ▼
  if (button == MouseButton_Left) {
    if (action == MouseAction_Press) {
      if (is_in_area(p, local_pos(), tri_size_)) {
        in_press_ = true;
        return { true, false };
      }
    } else {
      if (is_in_area(p, local_pos(), tri_size_)) {
        if (in_press_) {
          in_press_ = false;
          in_open_ = !in_open_;
          return { true, true };
        }
      }
    }
  }
  // ドラッグ.
  event_result r = drag_.on_mouse_button(local_pos(), p, button, action, mod);

  return r;
}

event_result window::on_cursor_move(const vec2& p)
{
  if (is_in_area(p, local_pos(), tri_size_)) {
    in_over_ = true;
  } else {
    in_over_ = false;
  }

  auto [r, diff] = drag_.on_cursor_move(p);
  if (r.accept) {
    set_local_pos(drag_.base_pos()  + diff);
    r.recalc_layout = true;
  }

  return r;
}

event_result window::on_cursor_enter(const vec2& p)
{
  if (is_in_area(p, local_pos(), tri_size_)) {
    in_over_ = true;
    return { true, false };
  }
  return { false, false };
}

event_result window::on_cursor_leave(const vec2& p)
{
  in_over_ = false;
  return { true, false };
}



group::group(const string_t& name, bool opened)
  : component_set(name), name_pos_{}, tri_size_{},
    in_open_(opened), in_press_(false), in_over_(false)
{
}

void group::draw(draw_context& cxt) const
{
  const system_property& prop = cxt.property;
  cxt.draw_font(name_pos_, prop.font_color, name());
  if (in_over_) {
    cxt.draw_rect(local_pos(), tri_size_, prop.active_color, prop.semiactive_color);
  }
  const color& cc =  in_press_ ? prop.active_color : prop.frame_color0;
  auto [x, y] = local_pos();
  if (in_open_) {
    cxt.draw_triangle(vec2{ x,                     y + tri_size_.y * 3.f / 8.f },
                      vec2{ x + tri_size_.x * .5f, y + tri_size_.y * 5.f / 8.f },
                      vec2{ x + tri_size_.x,       y + tri_size_.y * 3.f / 8.f },
                      cc);
    component_set::draw(cxt);
    draw_frame(cxt);
  } else {
    cxt.draw_triangle(vec2{ x + tri_size_.x / 4.f,       y + tri_size_.y / 4.f },
                      vec2{ x + tri_size_.x / 4.f,       y + tri_size_.y * 3.f / 4.f },
                      vec2{ x + tri_size_.x * 3.f / 4.f, y + tri_size_.y / 2.f },
                      cc);
  }
}

void group::draw_frame(draw_context& cxt) const
{
  // 枠線
  const system_property& prop = cxt.property;
  const color inside = { 0.f, 0.f, 0.f, 0.f };
  auto [x, y] = line_pos_;
  auto [w, h] = line_size_;
  auto [l, t] = line_lt_;
  float r = prop.round;
  const vertex vertex_array[] = {
    // TL
    { { l,     y     }, { .5f, .0f} },
    { { l,     y + r }, { .5f, .5f} },
    { { x,     t     }, { .0f, .5f} },
    { { x + r, t     }, { .5f, .5f} },
    // TR
    { { x + w - r, y     }, { .5f, .0f} },
    { { x + w - r, y + r }, { .5f, .5f} },
    { { x + w,     y     }, { .0f, .0f} },
    { { x + w,     y + r }, { .0f, .5f} },
    // BL
    { { x,     y + h - r }, { .0f, .5f} },
    { { x,     y + h     }, { .0f, .0f} },
    { { x + r, y + h - r }, { .5f, .5f} },
    { { x + r, y + h     }, { .5f, .0f} },
    // BR
    { { x + w - r, y + h - r }, { .5f, .5f} },
    { { x + w - r, y + h     }, { .5f, .0f} },
    { { x + w,     y + h - r }, { .0f, .5f} },
    { { x + w,     y + h     }, { .0f, .0f} },
  };
  static const uint32_t index_array[] = {
    0, 1, 4, 4, 1, 5, 4, 5, 6, 6, 5, 7,
    2, 8, 3, 3, 8, 10, 5, 12, 7, 7, 12, 14,
    8, 9, 10, 10, 9, 11, 10, 11, 12, 12, 11, 13, 12, 13, 14, 14, 13, 15 };
  cxt.draw(GL_TRIANGLES, vertex_array, countof(vertex_array),
           index_array, countof(index_array), cxt.property.frame_color0, inside);
}

vec2 group::calc_layout(calc_layout_context& cxt)
{
  const system_property& prop = cxt.property;
  tri_size_ = vec2{ prop.font_size / 2.f + prop.mergin, prop.font_size + prop.mergin * 2.f };
  rect name_area = cxt.font_renderer->get_area(prop.font_size, name());
  name_pos_ = local_pos() + vec2{ tri_size_.x, (float)prop.font_size };
  line_pos_ = local_pos() + vec2{ prop.mergin, prop.font_size / 2.f };
  vec2 pos = line_pos_ + vec2{ prop.mergin, prop.font_size + prop.mergin };
  vec2 child_size{};
  child_size = place_compoent_array(pos, cxt, child_array());
  child_size.x = std::max(child_size.x, tri_size_.x + name_area.w + prop.mergin);
  line_size_ = child_size + vec2{ prop.mergin, prop.mergin } + (pos - line_pos_);
  line_lt_.x = local_pos().x + tri_size_.x + name_area.w + prop.mergin;
  line_lt_.y = local_pos().y + tri_size_.y;
  set_size(tri_size_);

  if (in_open_) {
    return line_pos_ + line_size_ + vec2{ prop.mergin, prop.mergin } - local_pos();
  } else {
    return name_pos_ + vec2{ (float)name_area.w, (float)prop.mergin } - local_pos();
  }
}

bool group::make_event_handler_stack(const vec2& p, event_handler_stack_t& stk)
{
  bool ret = component::make_event_handler_stack(p, stk);
  if (in_open_) {
    for (auto c : child_array()) {
      ret = c->make_event_handler_stack(p, stk) || ret;
    }
  }
  return ret;
}

event_result group::on_mouse_button(const vec2& p, MouseButton button, MouseAction action, ModKey)
{
  if (button == MouseButton_Left) {
    if (action == MouseAction_Press) {
      if (is_in_area(p, local_pos(), tri_size_)) {
        in_press_ = true;
        return { true, false };
      }
    } else {
      if (is_in_area(p, local_pos(), tri_size_)) {
        if (in_press_) {
          in_press_ = false;
          in_open_ = !in_open_;
          return { true, true };
        }
      }
    }
  }

  return { false, false };
}

event_result group::on_cursor_enter(const vec2& p)
{
  if (is_in_area(p, local_pos(), tri_size_)) {
    in_over_ = true;
    return { true, false };
  }
  return { false, false };
}

event_result group::on_cursor_leave(const vec2& p)
{
  in_over_ = false;
  return { true, false };
}



button::button(const string_t& name, bool *notice)
  : component(name), name_pos_{}, area_size_{},
    in_press_(false), in_over_(false),
    notice_variable_(notice), notice_function_(0)
{
}

button::button(const string_t& name, callback_t notice)
  : component(name), name_pos_{}, area_size_{},
    in_press_(false), in_over_(false),
    notice_variable_(0), notice_function_(notice)
{
}

bool button::update()
{
  if (notice_variable_) {
    *notice_variable_ = false;
  }
  return false;
}

void button::draw(draw_context& cxt) const
{
  const system_property& prop = cxt.property;
  const color& col1 = in_press_ ? prop.active_color :
                      (in_over_ ? prop.semiactive_color : prop.frame_color1);
  cxt.draw_rect(local_pos(), area_size_, prop.frame_color0, col1);
  cxt.draw_font(name_pos_, prop.font_color, name());
}

vec2 button::calc_layout(calc_layout_context& cxt)
{
  const system_property& prop = cxt.property;
  rect name_area = cxt.font_renderer->get_area(prop.font_size, name());
  area_size_.x = name_area.w + prop.mergin * 2.f;
  area_size_.y = prop.font_size + prop.mergin * 2.f;
  name_pos_.x = local_pos().x + prop.mergin;
  name_pos_.y = local_pos().y + int(prop.font_size + prop.mergin * 2 - name_area.h) / 2 - name_area.y;
  set_size(area_size_);

  return area_size_;
}

event_result button::on_mouse_button(const vec2& p, MouseButton button, MouseAction action, ModKey mod)
{
  if ((button == MouseButton_Left)) {
    if (action == MouseAction_Press) {
      if (is_in_area(p, local_pos(), area_size_)) {
        in_press_ = true;
        return { true, false };
      }
    } else {
      if (is_in_area(p, local_pos(), area_size_)) {
        if (in_press_) {
          in_press_ = false;
          // クリック認定.
          if (notice_variable_) {
            *notice_variable_ = true;
          }
          if (notice_function_) {
            notice_function_();
          }
          return { true, false };
        }
      }
      in_press_ = false;
    }
  }
  return { false, false };
}

event_result button::on_cursor_enter(const vec2&)
{
  in_over_ = true;
  return { true, false };
}

event_result button::on_cursor_leave(const vec2&)
{
  in_press_ = false;
  in_over_ = false;
  return { true, false };
}



combo_box::combo_box(const string_t& name, int *value)
  : component(name),
    name_pos_{},
    in_open_(false), in_press_(false), in_over_(false), cur_index_(0),
    notice_variable_(value)
{
}

void combo_box::draw(draw_context& cxt) const
{
  const system_property& prop = cxt.property;
  const color& cc =  (in_press_ || in_open_) ? prop.active_color : prop.frame_color0;
  cxt.draw_font(name_pos_, prop.font_color, name());
  cxt.draw_rect(tri_pos_, tri_size_, prop.frame_color0, (in_over_ || in_open_) ? prop.semiactive_color : prop.frame_color1);
  cxt.draw_triangle(vec2{ tri_pos_.x + prop.mergin,                     tri_pos_.y + tri_size_.y * 3.f / 8.f },
                    vec2{ tri_pos_.x + tri_size_.x * .5f,               tri_pos_.y + tri_size_.y * 5.f / 8.f },
                    vec2{ tri_pos_.x + tri_size_.x * 1.f - prop.mergin, tri_pos_.y + tri_size_.y * 3.f / 8.f },
                    cc);
  cxt.draw_rect(item_pos_, item_size_, prop.frame_color0, prop.frame_color1);
  cxt.draw_font(item_font_pos_, prop.font_color, item_array_[*notice_variable_]);
  if (in_open_) {
    cxt.draw_rect(item_list_pos_, item_list_size_, prop.frame_color0, prop.semiactive_color);
    if (cur_index_ >= 0) {
      cxt.draw_rect(item_list_pos_ + vec2{ 0.f, item_height_ * cur_index_ },
                    vec2{ item_list_size_.x, item_height_ },
                    prop.frame_color0, prop.active_color);
    }
    vec2 pos = item_list_font_pos_;
    for (const auto& s : item_array_) {
      cxt.draw_font(pos, prop.font_color, s);
      pos.y += item_height_;
    }
  }
}

vec2 combo_box::calc_layout(calc_layout_context& cxt)
{
  const system_property& prop = cxt.property;
  rect name_area = cxt.font_renderer->get_area(prop.font_size, name());
  name_pos_ = local_pos() + vec2{ 0.f, prop.font_size + prop.mergin };
  float max_width = 0.f;
  for (const auto& c : item_array_) {
    rect area = cxt.font_renderer->get_area(prop.font_size, c);
    max_width = std::max(max_width, (float)area.w);
  }
  item_pos_ = local_pos() + vec2{ name_area.w + prop.mergin, 0.f };
  item_size_ = vec2{ max_width + prop.mergin * 2.f, prop.font_size + prop.mergin * 2.f };
  item_font_pos_ = item_pos_ + vec2{ prop.mergin, prop.font_size + prop.mergin };
  tri_pos_ = item_pos_ + vec2{ item_size_.x- 1.f, 0.f };
  tri_size_ = vec2{ item_size_.y / 2.f + prop.mergin * 2.f, item_size_.y };
  vec2 size = tri_pos_ + tri_size_ - local_pos();

  if (in_open_) {
    item_height_ = prop.font_size + prop.mergin * 2.f;
    item_list_pos_ = item_pos_ + vec2{ 0.f, size.y - 1.f };
    item_list_size_ = vec2{ item_size_.x, item_height_ * item_array_.size() };
    item_list_font_pos_ = item_list_pos_ + vec2{ prop.mergin, prop.font_size + prop.mergin };
    set_size({size.x, size.y + item_list_size_.y});
  } else {
    set_size(size);
  }
  return size;
}

event_result combo_box::on_mouse_button(const vec2& p, MouseButton button, MouseAction action, ModKey)
{
  if (item_array_.size() < 2) {
    return { false, false };
  }
  if (in_open_) {
    if (button == MouseButton_Left) {
      if (action == MouseAction_Press) {
        // ▼
        if (is_in_area(p, item_pos_, vec2{ item_size_.x + tri_size_.x, item_size_.y })) {
          in_press_ = true;
          return { true, false };
        }
        // アイテムリスト.
        if (is_in_area(p, item_list_pos_, item_list_size_)) {
          in_press_ = true;
          return { true, false };
        }
      } else {
        // ▼.
        if (is_in_area(p, item_pos_, vec2{ item_size_.x + tri_size_.x, item_size_.y })) {
          if (in_press_) {
            in_open_ = false;
            in_press_ = false;
            return { true, true };
          } else {
            return { true, false };
          }
        }
        // アイテムリスト.
        if (is_in_area(p, item_list_pos_, item_list_size_)) {
          if (in_press_) {
            // 確定.
            if (notice_variable_) {
              if (cur_index_ >= 0) {
                *notice_variable_ = cur_index_;
              }
            }
            in_open_ = false;
            in_press_ = false;
            in_over_ = false;
            return { true, true };
          }
        }
      }
    }
  } else {
    // アイテムか▼押されたら開く.
    if ((button == MouseButton_Left) && (action == MouseAction_Press)) {
      if (is_in_area(p, item_pos_, vec2{ item_size_.x + tri_size_.x, item_size_.y })) {
        in_open_ = true;
        return { true, true };
      }
    }
  }

  return { false, false };
}

event_result combo_box::on_cursor_move(const vec2& p)
{
  if (in_open_) {
    cur_index_ = get_item_list_index(p);
    if (cur_index_ >= 0) {
      return { true, false };
    }
  }
  return { false, false };
}

event_result combo_box::on_cursor_enter(const vec2&)
{
  in_over_ = true;
  return { true, false };
}

event_result combo_box::on_cursor_leave(const vec2&)
{
  in_over_ = false;
  return { true, false };
}

event_result combo_box::on_lost_focus()
{
  in_open_ = false;
  in_press_ = false;
  return { true, true };
}

int combo_box::add_item(const string_t& s)
{
  item_array_.push_back(s);
  return (int)item_array_.size();
}

void combo_box::remove_item(int index)
{
  item_array_.erase(item_array_.begin() + index);
}

void combo_box::clear_item()
{
  item_array_.clear();
}

int combo_box::get_item_list_index(const vec2& p)
{
  size_t num = item_array_.size();
  for (size_t i=0; i<num; ++i) {
    if (is_in_area(p,
                   item_list_pos_ + vec2{ 0.f, i * item_height_ },
                   vec2{ item_list_size_.x, item_height_ })) {
      return (int)i;
    }
  }
  return -1;
}



check_box::check_box(const string_t& name, bool *value)
  : component(name), notice_variable_(value),
    in_press_(false), in_over_(false)
{
}

void check_box::draw(draw_context& cxt) const
{
  const system_property& prop = cxt.property;
  const color& border = (in_over_ || in_press_) ? prop.active_color : prop.frame_color0;
  const color& inside = (*notice_variable_ || in_press_) ? prop.active_color :
                        in_over_ ? prop.semiactive_color : prop.frame_color1;
  cxt.draw_rect(box_pos_, box_size_, border, inside);
  cxt.draw_font(name_pos_, prop.font_color, name());
}

vec2 check_box::calc_layout(calc_layout_context& cxt)
{
  const system_property& prop = cxt.property;
  box_pos_ = local_pos();
  box_size_ = vec2{ (float)prop.font_size, (float)prop.font_size };
  rect name_area = cxt.font_renderer->get_area(prop.font_size, name());
  name_pos_ = local_pos() + vec2{ box_size_.x + prop.mergin, (float)-name_area.y };
  set_size(box_pos_ + box_size_ - local_pos());

  return name_pos_ + vec2{ prop.mergin + name_area.w, prop.mergin } - local_pos();
}

event_result check_box::on_mouse_button(const vec2& p, MouseButton button, MouseAction action, ModKey)
{
  if (button == MouseButton_Left) {
    if (action == MouseAction_Press) {
      if (is_in_area(p, box_pos_, box_size_)) { 
        in_press_ = true;
        return { true, false };
      }
    } else {
      if (is_in_area(p, box_pos_, box_size_)) {
        if (in_press_) {
          in_press_ = false;
          *notice_variable_ = !*notice_variable_;
          return { true, false };
        }
      }
    }
  }
  return { false, false };
}

event_result check_box::on_cursor_enter(const vec2& p)
{
  if (is_in_area(p, box_pos_, box_size_)) {
    in_over_ = true;
    return { true, false };
  }
  
  return { false, false };
}

event_result check_box::on_cursor_leave(const vec2&)
{
  in_over_ = false;

  return { true, false };
}



radio_button::radio_button(const string_t& name, int *variable, int value)
  : component(name), in_press_(false), in_over_(false),
    notice_variable_(variable), value_(value)
{
}

void radio_button::draw(draw_context& cxt) const
{
  const system_property& prop = cxt.property;
  const color& border = (in_over_ || in_press_) ? prop.active_color : prop.frame_color0;
  const color& inside = ((*notice_variable_ == value_) || in_press_) ? prop.active_color :
                        in_over_ ? prop.semiactive_color : prop.frame_color1;
  cxt.draw_font(name_pos_, prop.font_color, name());
  // ◆
  const vertex vertex_array[] = {
    { { box_pos_.x + box_size_.x / 2.f, box_pos_.y + box_size_.y / 2.f }, { .5f, .5f } },
    { { box_pos_.x + box_size_.x / 2.f, box_pos_.y                     }, { .5f, .0f } },
    { { box_pos_.x,                     box_pos_.y + box_size_.y / 2.f }, { .5f, .0f } },
    { { box_pos_.x + box_size_.x / 2.f, box_pos_.y + box_size_.y       }, { .5f, .0f } },
    { { box_pos_.x + box_size_.x,       box_pos_.y + box_size_.y / 2.f }, { .5f, .0f } },
  };
  static const uint32_t index_array[] = {
    0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 1
  };
  cxt.draw(GL_TRIANGLES, vertex_array, countof(vertex_array), index_array, countof(index_array), border, inside);
}

vec2 radio_button::calc_layout(calc_layout_context& cxt)
{
  const system_property& prop = cxt.property;
  box_pos_ = local_pos();
  box_size_ = vec2{ (float)prop.font_size, (float)prop.font_size };
  rect name_area = cxt.font_renderer->get_area(prop.font_size, name());
  name_pos_ = local_pos() + vec2{ box_size_.x + prop.mergin, (float)-name_area.y };
  set_size(box_pos_ + box_size_ - local_pos());

  return name_pos_ + vec2{ prop.mergin + name_area.w, prop.mergin } - local_pos();
}

event_result radio_button::on_mouse_button(const vec2& p, MouseButton button, MouseAction action, ModKey)
{
  if (button == MouseButton_Left) {
    if (action == MouseAction_Press) {
      if (is_in_area(p, box_pos_, box_size_)) {
        in_press_ = true;
        return { true, false };
      }
    } else {
      if (is_in_area(p, box_pos_, box_size_)) {
        if (in_press_) {
          in_press_ = false;
          *notice_variable_ = value_;
          return { true, false };
        }
      }
    }
  }
  return { false, false };
}

event_result radio_button::on_cursor_enter(const vec2& p)
{
  if (is_in_area(p, box_pos_, box_size_)) {
    in_over_ = true;
    return { true, false };
  }

  return { false, false };
}

event_result radio_button::on_cursor_leave(const vec2&)
{
  in_over_ = false;

  return { true, false };
}



label::label(const string_t& name)
  : component(name), name_pos_{}
{
}

void label::draw(draw_context& cxt) const
{
  const system_property& prop = cxt.property;
  cxt.draw_font(name_pos_, prop.font_color, name());
}

vec2 label::calc_layout(calc_layout_context& cxt)
{
  const system_property& prop = cxt.property;
  rect name_area = cxt.font_renderer->get_area(prop.font_size, name());
  name_pos_ = local_pos() + vec2{ 0.f, (float)-name_area.y };

  return vec2{ (float)name_area.w, (float)prop.font_size };
}


const float text_box::DEFAULT_WIDTH = 120.f;

text_box::text_box(const string_t& name, string_function_t f, float width)
  : component(name), width_(width), string_function_(f)
{
}

bool text_box::update()
{
  string_t str = string_function_();
  bool r = str_ != str;
  str_ = str;

  return r;
}

void text_box::draw(draw_context& cxt) const
{
  const auto& prop = cxt.property;
  cxt.draw_rect(local_pos(), text_size_, prop.frame_color0, color::zero());
  cxt.draw_font(name_pos_, prop.font_color, name());
  cxt.draw_font(text_font_pos_, prop.font_color, str_);
}

vec2 text_box::calc_layout(calc_layout_context& cxt)
{
  const auto& prop = cxt.property;
  rect text_area = cxt.font_renderer->get_area(prop.font_size, str_);
  float width = std::max(width_, text_area.w + prop.mergin * 2.f);
  text_size_ = vec2{ width, prop.font_size + prop.mergin * 2.f };
  text_font_pos_ = local_pos() + vec2{ prop.mergin, prop.font_size + prop.mergin };
  name_pos_ = local_pos() + vec2{ text_size_.x + prop.mergin, prop.font_size + prop.mergin };
  rect name_area = cxt.font_renderer->get_area(prop.font_size, name());

  return name_pos_ + vec2{ name_area.w, prop.mergin } - local_pos();
}


const float slider_base::DEFAULT_WIDTH = 120.f;

slider_base::slider_base(const string_t& name, float width)
  : component(name), width_(width), in_over_(false)
{
}

void slider_base::draw(draw_context& cxt) const
{
  const auto& prop = cxt.property;
  cxt.draw_font(name_pos_, prop.font_color, name());
  const color& border_color = in_over_ ? prop.semiactive_color : prop.frame_color0;
  const color& value_color = drag_.in_drag() ? prop.active_color : prop.semiactive_color;
  cxt.draw_rect(base_pos_, base_size_, border_color, color::zero());
  if (value_size_.x > 0) {
    cxt.draw_rect(base_pos_, value_size_, border_color, value_color);
  }
  cxt.draw_font(value_font_pos_, prop.font_color, value_str());
}

vec2 slider_base::calc_layout(calc_layout_context& cxt)
{
  const auto& prop = cxt.property;
  base_pos_ = local_pos();
  base_size_ = vec2{ width_, prop.font_size + prop.mergin * 2.f };
  value_size_ = vec2{ value_width(width_), base_size_.y };
  rect value_area = cxt.font_renderer->get_area(prop.font_size, value_str());
  value_font_pos_ = vec2{ roundup(base_pos_.x + base_size_.x / 2.f - value_area.w / 2.f),
                          base_pos_.y + prop.font_size + prop.mergin };
  rect name_area = cxt.font_renderer->get_area(prop.font_size, name());
  name_pos_ = base_pos_ + vec2{ base_size_.x + prop.mergin, prop.font_size + prop.mergin };

  set_size(name_pos_ + vec2{ (float)name_area.w, prop.mergin } - local_pos());
  drag_.set_area(base_pos_, base_size_);

  return size();
}

event_result slider_base::on_mouse_button(const vec2& p, MouseButton button, MouseAction action, ModKey mod)
{
  auto r = drag_.on_mouse_button(value_size_, p, button, action, mod);
  return r;
}

event_result slider_base::on_cursor_move(const vec2& p)
{
  auto [r, diff] = drag_.on_cursor_move(p);
  if (r.accept) {
    set_value((drag_.base_pos().x + diff.x) / width_);
    r.recalc_layout = true;
  }
  return r;
}

event_result slider_base::on_cursor_enter(const vec2& p)
{
  in_over_ = true;
  return { true, false };
}

event_result slider_base::on_cursor_leave(const vec2&)
{
  in_over_ = false;
  return { true, false };
}

event_result slider_base::on_lost_focus()
{
  return drag_.on_lost_focus();
}


const float numeric_up_down_base::DEFAULT_WIDTH = 120.f;

numeric_up_down_base::numeric_up_down_base(const string_t& name, float width)
  : component(name), width_(width),
    in_over_up_(false), in_over_down_(false), in_over_textedit_(false),
    in_press_up_(false), in_press_down_(false), in_press_textedit_(false),
    in_textedit_(false)
{
}

void numeric_up_down_base::draw(draw_context& cxt) const
{
  const auto& prop = cxt.property;
  cxt.draw_rect(local_pos(), value_size_,
                in_textedit_ ? prop.active_color :
                in_over_textedit_ ? prop.semiactive_color : prop.frame_color0, color::zero());
  textedit_.draw_font(value_font_pos_, cxt);
  if (in_textedit_) {
    textedit_.draw_cursor(value_font_pos_, cxt);
  }
  cxt.draw_rect(up_pos_, up_size_, prop.frame_color0, in_over_up_ ? prop.semiactive_color : color::zero());
  cxt.draw_triangle(up_pos_ + vec2{ up_size_.x / 2.f,       prop.mergin / 2.f },
                    up_pos_ + vec2{ up_size_.x * 1.f / 4.f, up_size_.y - prop.mergin / 2.f },
                    up_pos_ + vec2{ up_size_.x * 3.f / 4.f, up_size_.y - prop.mergin / 2.f },
                    in_press_up_ ? prop.active_color : prop.frame_color0);
  cxt.draw_rect(down_pos_, down_size_, prop.frame_color0, in_over_down_ ? prop.semiactive_color : color::zero());
  cxt.draw_triangle(down_pos_ + vec2{ down_size_.x / 2.f,       down_size_.y - prop.mergin / 2.f },
                    down_pos_ + vec2{ down_size_.x * 3.f / 4.f, prop.mergin / 2.f },
                    down_pos_ + vec2{ down_size_.x * 1.f / 4.f, prop.mergin / 2.f },
                    in_press_down_ ? prop.active_color : prop.frame_color0);
  cxt.draw_font(name_pos_, prop.font_color, name());
}

vec2 numeric_up_down_base::calc_layout(calc_layout_context& cxt)
{
  const auto& prop = cxt.property;
  rect value_area = cxt.font_renderer->get_area(prop.font_size, textedit_.str());
  value_size_.x = std::max(value_area.w + prop.mergin * 2.f, width_);
  value_size_.y = prop.font_size + prop.mergin * 2.f;
  value_font_pos_ = local_pos() + vec2{ value_size_.x - value_area.w - prop.mergin, prop.font_size + prop.mergin };
  up_pos_ = local_pos() + vec2{ value_size_.x - 1.f, 0.f };
  up_size_ = vec2{ value_size_.y, value_size_.y / 2.f };
  down_pos_ = up_pos_ + vec2{ 0.f, up_size_.y };
  down_size_ = up_size_;
  name_pos_ = up_pos_ + vec2{ up_size_.x + prop.mergin, prop.font_size + prop.mergin };
  rect name_area = cxt.font_renderer->get_area(prop.font_size, name());

  set_size(vec2{ value_size_.x + up_size_.x, value_size_.y });

  return vec2{ name_pos_.x + name_area.w, name_pos_.y + prop.mergin } - local_pos();
}

bool numeric_up_down_base::make_event_handler_stack(const vec2& p, event_handler_stack_t& stk)
{
  bool r = false;
  if (is_in_area(p, up_pos_, up_size_)) {
    stk.push({shared_from_this(), 0});
    r = true;
  }
  if (is_in_area(p, down_pos_, down_size_)) {
    stk.push({shared_from_this(), 1});
    r = true;
  }
  if (is_in_area(p, local_pos(), value_size_)) {
    stk.push({shared_from_this(), 2});
    r = true;
  }
  return r;
}

event_result numeric_up_down_base::on_mouse_button(const vec2& p, MouseButton button, MouseAction action, ModKey)
{
  if (button == MouseButton_Left) {
    if (action == MouseAction_Press) {
      if (is_in_area(p, up_pos_, up_size_)) {
        in_press_up_ = true;
        end_textedit();
        return { true, false };
      }
      if (is_in_area(p, down_pos_, down_size_)) {
        in_press_down_ = true;
        end_textedit();
        return { true, false };
      }
      if (is_in_area(p, local_pos(), value_size_)) {
        in_press_textedit_ = true;
        return { true, false };
      }
    } else {
      if (is_in_area(p, up_pos_, up_size_)) {
        if (in_press_up_) {
          in_press_up_ = false;
          increment();
        }
        return { true, false };
      }
      if (is_in_area(p, down_pos_, down_size_)) {
        if (in_press_down_) {
          in_press_down_ = false;
          decrement();
        }
        return { true, false };
      }
      if (is_in_area(p, local_pos(), value_size_)) {
        in_press_textedit_ = false;
        in_textedit_ = true;
        textedit_.set_cursor(0);
        return { true, false };
      }
    }
  }
  return { false, false };
}

event_result numeric_up_down_base::on_cursor_move(const vec2&)
{
  return { false, false };
}

event_result numeric_up_down_base::on_cursor_enter(const vec2& p)
{
  if (is_in_area(p, up_pos_, up_size_)) {
    in_over_up_ = true;
    return { true, false };
  }
  if (is_in_area(p, down_pos_, down_size_)) {
    in_over_down_ = true;
    return { true, false };
  }
  if (is_in_area(p, local_pos(), value_size_)) {
    in_over_textedit_ = true;
    return { true, false };
  }
  return { false, false };
}

event_result numeric_up_down_base::on_cursor_leave(const vec2& p)
{
  if (!is_in_area(p, up_pos_, up_size_)) {
    in_over_up_ = false;
  }
  if (!is_in_area(p, down_pos_, down_size_)) {
    in_over_down_ = false;
  }
  if (!is_in_area(p, local_pos(), value_size_)) {
    in_over_textedit_ = false;
    return { true, false };
  }

  return { true, false };
}

event_result numeric_up_down_base::on_input_key(int key, int scancode, KeyAction action, ModKey mod)
{
  if (in_textedit_) {
    auto r = textedit_.on_input_key(key, scancode, action, mod);
    if (r.accept) {
      end_textedit();
    }
    return { true, r.recalc_layout };
  }
  return { false, false };
}

event_result numeric_up_down_base::on_input_char(char_t code)
{
  if (in_textedit_) {
    if (is_accept_char(code)) {
      return textedit_.on_input_char(code);
    }
  }
  return { false, false };
}

event_result numeric_up_down_base::on_lost_focus()
{
  if (in_textedit_) {
    end_textedit();
  }
  return { true, false };
}

void numeric_up_down_base::end_textedit()
{
  value_str(textedit_.str());
  in_textedit_ = false;
}



constructor::constructor(gui::system::ptr_t p)
  : system_(p)
{
}

constructor::~constructor()
{
  system_->calc_layout();
}

void constructor::same_line()
{
  if (cur_) {
    cur_->set_layout_way(LayoutWay_Horizon);
  }
}

} // end of namespace gui
