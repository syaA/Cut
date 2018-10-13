
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

vec2 layout_vertical(vec2 pos, calc_layout_context& cxt, std::vector<component::ptr_t>& component_array)
{
  const system_property& prop = cxt.property;
  auto orig = pos;
  float width = .0f;
  for (auto c : component_array) {
    c->set_local_pos(pos);
    auto diff = c->calc_layout(cxt);
    pos.y += diff.y + prop.mergin;
    width = std::max(width, diff.x);
  }
  return { width, pos.y - orig.y };
}

} // end of namespace



void draw_context::draw(const vertex* vertex_array, int vertex_cnt,
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

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, gui_tex->texture_globj());
  glBindSampler(0, gui_tex->sampler_globj());
  gui_shader->set_uniform("gui_sampler", 0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  glDrawElements(GL_TRIANGLES,
                 index_cnt,
                 GL_UNSIGNED_INT,
                 index_array);
}

void draw_context::draw_triangle(const vec2& p0, const vec2& p1, const vec2& p2, const color& c)
{
  float r = 10.f;
  const vertex vertex_array[] = {
    { { p0.x, p0.y }, { 1.f, 1.f} },
    { { p1.x, p1.y }, { 1.f, 1.f} },
    { { p2.x, p2.y }, { 1.f, 1.f} },
  };
  static const uint32_t index_array[] = {
    0, 1, 2
  };
  draw(vertex_array, countof(vertex_array), index_array, countof(index_array), c, c);
}

void draw_context::draw_rect(const vec2& pos, const vec2& size, const color& c0, const color& c1)
{
  auto x = pos.x;
  auto y = pos.y;
  auto w = size.x;
  auto h = size.y;
  float r = 10.f;
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
  draw(vertex_array, countof(vertex_array), index_array, countof(index_array), c0, c1);
}

void draw_context::draw_font(const vec2& pos, const color& c, const string& str)
{
  font_renderer->render(pos, { property.font_size, property.font_size }, c, str);
}



component::component(const string& name)
  : name_(name), local_pos_(0.f, 0.f), size_(0.f, 0.f)
{
}

void component::make_event_handler_stack(const vec2& p, event_handler_stack_t& stk)
{
  if (is_in_area(p, local_pos(), size())) {
    stk.push(shared_from_this());
  }
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

event_result component::on_input_char(char16_t code)
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


void component_set::update()
{
  for (const auto& c : child_array_) {
    c->update();
  }
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
  if (focused) {
    focused->draw(cxt);
  }
}

vec2 component_set::calc_layout(calc_layout_context& cxt)
{
  set_size(layout_vertical(local_pos(), cxt, child_array()));
  return size();
}

void component_set::make_event_handler_stack(const vec2& p, event_handler_stack_t& stk)
{
  component::make_event_handler_stack(p, stk);
  for (auto c : child_array()) {
    c->make_event_handler_stack(p, stk);
  }
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



system::system(shader::ptr_t s, texture::ptr_t t, font::renderer::ptr_t f)
  : component_set(u"root"),
    shader_(s), texture_(t), font_renderer_(f),
    prev_cursor_pos_(0.f)
{
  glGenBuffers(1, &vertex_buffer_);

  // デフォルトプロパティ.
  property_.font_color = color(0.f, 0.f, 0.f, 1.f);
  property_.font_size = 16;
  property_.frame_color0 = color(0.f, 0.f, 0.f, 1.f);
  property_.frame_color1 = color(1.f, 1.f, 1.f, .7f);
  property_.active_color = color(1.f, 0.65f, 0.0f, 1.0f);
  property_.semiactive_color = color(0.99f, 0.96f, 0.75f, 1.f);
  property_.mergin = 5.f;
}

system::~system()
{
  glDeleteBuffers(1, &vertex_buffer_);
}

void system::update()
{
  component_set::update();
}

void system::draw()
{
  glDisable(GL_DEPTH_TEST);

  font_renderer_->set_screen_size((int)screen_size_.x, (int)screen_size_.y);

  draw_context cxt = {
    shader_, texture_, vertex_buffer_, font_renderer_, screen_size_, property_, focused_.lock() };

  component_set::draw(cxt);
}

void system::calc_layout()
{
  calc_layout_context cxt = {
    font_renderer_, screen_size_, property_ };
  for (auto c : child_array()) {
    c->calc_layout(cxt);
  }
}

bool system::on_mouse_button_root(vec2 p, MouseButton button, MouseAction action, ModKey mod)
{
  p = clamp(p, vec2(0.f), screen_size_);
  event_handler_stack_t stk;
  make_event_handler_stack(p, stk);
  event_result r;
  while (!stk.empty()) {
    r = stk.top()->on_mouse_button(p, button, action, mod);
    if (r.accept) {
      set_focus(stk.top());
      break;
    }
    stk.pop();
  }
  if (!r.accept) {
    set_focus(0);
  }

  if (r.recalc_layout) {
    calc_layout();
  }
  return r.accept;
}

bool system::on_cursor_move_root(vec2 p)
{
  p = clamp(p, vec2(0.f), screen_size_);
  event_handler_stack_t cur_stk, pre_stk;
  make_event_handler_stack(p, cur_stk);
  make_event_handler_stack(prev_cursor_pos_, pre_stk);

  // enter
  event_result r_enter;
  while (!cur_stk.empty()) {
    auto c = cur_stk.top();
    if (!is_in_area(prev_cursor_pos_, c->local_pos(), c->size())) {
      r_enter = c->on_cursor_enter(p);
    }
    cur_stk.pop();
  }
  // leave
  event_result r_leave;
  while (!pre_stk.empty()) {
    auto c = pre_stk.top();
    if (!is_in_area(p, c->local_pos(), c->size())) {
      r_leave = c->on_cursor_leave(p);
    }
    pre_stk.pop();
  }
  // move
  event_result r_move;
  if (auto c = focus()) {
    r_move = c->on_cursor_move(p);
  }

  if (r_enter.recalc_layout || r_leave.recalc_layout || r_move.recalc_layout) {
    calc_layout();
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
  p = clamp(p, vec2(0.f), screen_size_);
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
    calc_layout();
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
    calc_layout();
  }
  return r.accept;
}

bool system::on_input_char_root(char16_t code)
{
  event_result r;
  if (auto c = focus()) {
    r = c->on_input_char(code);
  }
  if (r.recalc_layout) {
    calc_layout();
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
    calc_layout();
  }
  focused_ = p;
}

system::component_ptr_t system::focus() const
{
  return focused_.lock();
}


drag_control::drag_control()
  : area_pos_(0.f), area_size_(0.f), on_drag_(false)
{
}

event_result drag_control::on_mouse_button(component::ptr_t c, const vec2& p, MouseButton button, MouseAction action, ModKey)
{
  if ((button == MouseButton_Left)) {
    if (action == MouseAction_Press) {
      if (is_in_area(p, area_pos_, area_size_)) {
        cur_pos_ = c->local_pos();
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

event_result drag_control::on_cursor_move(component::ptr_t c, const vec2& p)
{
  if (on_drag_) {
    vec2 diff = p - start_pos_;
    c->set_local_pos(cur_pos_ + diff);
    return { true, true };
  }
  return { false, false };
}



window::window(const string& name)
  : component_set(name)
{
}

void window::draw(draw_context& cxt) const
{
  cxt.draw_rect(local_pos(), size(), cxt.property.frame_color0, cxt.property.frame_color1);
  cxt.draw_font(name_pos_, cxt.property.font_color, name());

  component_set::draw(cxt);
}

vec2 window::calc_layout(calc_layout_context& cxt)
{
  const system_property& prop = cxt.property;
  rect name_area = cxt.font_renderer->get_area(prop.font_size, name());
  float width = name_area.w + prop.mergin * 2.f;
  auto pos = local_pos();
  name_pos_ = pos + vec2(prop.mergin, prop.mergin - name_area.y);
  pos += vec2(prop.mergin, prop.mergin - name_area.y + prop.mergin);
  vec2 size = layout_vertical(pos, cxt, child_array());
  size.x = std::max(width, size.x + prop.mergin * 2.f);
  size.y = (pos.y + size.y) - local_pos().y;
  set_size(size);

  drag_.set_area(local_pos(), vec2(size.x, name_area.h + prop.mergin));

  return size;
}

event_result window::on_mouse_button(const vec2& p, MouseButton button, MouseAction action, ModKey mod)
{
  event_result r = drag_.on_mouse_button(shared_from_this(), p, button, action, mod);
  if (!r.accept) {
    r = component_set::on_mouse_button(p, button, action, mod);
  }
  return r;
}

event_result window::on_cursor_move(const vec2& p)
{
  event_result r = drag_.on_cursor_move(shared_from_this(), p);
  return r;
}



button::button(const string& name, bool *notice)
  : component(name), name_pos_(0.f), area_pos_(0.f), area_size_(0.f),
    in_press_(false), in_over_(false),
    notice_variable_(notice), notice_function_(0)
{
}

button::button(const string& name, callback_t notice)
  : component(name), name_pos_(0.f), area_pos_(0.f), area_size_(0.f),
    in_press_(false), in_over_(false),
    notice_variable_(0), notice_function_(notice)
{
}

void button::update()
{
  if (notice_variable_) {
    *notice_variable_ = false;
  }
}

void button::draw(draw_context& cxt) const
{
  const system_property& prop = cxt.property;
  const color& col1 = in_press_ ? prop.active_color :
                      (in_over_ ? prop.semiactive_color : prop.frame_color1);
  cxt.draw_rect(area_pos_, area_size_, prop.frame_color0, col1);
  cxt.draw_font(name_pos_, prop.font_color, name());
}

vec2 button::calc_layout(calc_layout_context& cxt)
{
  const system_property& prop = cxt.property;
  rect name_area = cxt.font_renderer->get_area(prop.font_size, name());
  area_pos_ = local_pos();
  area_size_.x = name_area.w + prop.mergin * 2.f;
  area_size_.y = name_area.h + prop.mergin * 2.f;
  name_pos_ = area_pos_ + vec2(prop.mergin, prop.mergin - name_area.y);

  set_size(area_size_);
  return area_size_;
}

event_result button::on_mouse_button(const vec2& p, MouseButton button, MouseAction action, ModKey mod)
{
  if ((button == MouseButton_Left)) {
    if (action == MouseAction_Press) {
      if (is_in_area(p, area_pos_, area_size_)) {
        in_press_ = true;
        return { true, false };
      }
    } else {
      if (is_in_area(p, area_pos_, area_size_)) {
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



combo_box::combo_box(const string& name, int *value)
  : component(name),
    name_pos_(0.f),
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
  cxt.draw_triangle(vec2(tri_pos_.x + prop.mergin,                     tri_pos_.y + tri_size_.y * 3.f / 8.f),
                    vec2(tri_pos_.x + tri_size_.x * .5f,               tri_pos_.y + tri_size_.y * 5.f / 8.f),
                    vec2(tri_pos_.x + tri_size_.x * 1.f - prop.mergin, tri_pos_.y + tri_size_.y * 3.f / 8.f),
                    cc);
  cxt.draw_rect(item_pos_, item_size_, prop.frame_color0, prop.frame_color1);
  cxt.draw_font(item_font_pos_, prop.font_color, item_array_[*notice_variable_]);
  if (in_open_) {
    cxt.draw_rect(item_list_pos_, item_list_size_, prop.frame_color0, prop.semiactive_color);
    if (cur_index_ >= 0) {
      cxt.draw_rect(item_list_pos_ + vec2(0.f, item_height_ * cur_index_),
                    vec2(item_list_size_.x, item_height_),
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
  name_pos_ = local_pos() + vec2(0.f, prop.font_size + prop.mergin);
  float max_width = 0.f;
  for (const auto& c : item_array_) {
    rect area = cxt.font_renderer->get_area(prop.font_size, c);
    max_width = std::max(max_width, (float)area.w);
  }
  item_pos_ = local_pos() + vec2(name_area.w + prop.mergin, 0.f);
  item_size_ = vec2(max_width + prop.mergin * 2.f, prop.font_size + prop.mergin * 2.f);
  item_font_pos_ = item_pos_ + vec2(prop.mergin, prop.font_size + prop.mergin);
  tri_pos_ = item_pos_ + vec2(item_size_.x- 1.f, 0.f);
  tri_size_ = vec2(item_size_.y / 2.f + prop.mergin * 2.f, item_size_.y);
  vec2 size = tri_pos_ + tri_size_ - local_pos();

  if (in_open_) {
    item_height_ = prop.font_size + prop.mergin;
    item_list_pos_ = item_pos_ + vec2(0.f, size.y - 1.f);
    item_list_size_ = vec2(item_size_.x, item_height_ * item_array_.size());;
    item_list_font_pos_ = item_list_pos_ + vec2(prop.mergin, (float)prop.font_size);
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
        if (is_in_area(p, item_pos_, vec2(item_size_.x + tri_size_.x, item_size_.y))) {
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
        if (is_in_area(p, item_pos_, vec2(item_size_.x + tri_size_.x, item_size_.y))) {
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
      if (is_in_area(p, item_pos_, vec2(item_size_.x + tri_size_.x, item_size_.y))) {
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

int combo_box::add_item(const string& s)
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
                   item_list_pos_ + vec2(0.f, i * item_height_),
                   vec2(item_list_size_.x, item_height_))) {
      return (int)i;
    }
  }
  return -1;
}



check_box::check_box(const string& name, bool *value)
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
  box_pos_ = local_pos() + vec2(prop.mergin);
  box_size_ = vec2((float)prop.font_size);
  rect name_area = cxt.font_renderer->get_area(prop.font_size, name());
  name_pos_ = local_pos() + vec2(box_size_.x + 2.f * prop.mergin, prop.mergin - name_area.y);
  set_size(box_pos_ + box_size_ - local_pos());

  return name_pos_ + vec2(prop.mergin + name_area.w, prop.mergin) - local_pos();
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



radio_button::radio_button(const string& name, int *variable, int value)
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
  cxt.draw(vertex_array, countof(vertex_array), index_array, countof(index_array), border, inside);
}

vec2 radio_button::calc_layout(calc_layout_context& cxt)
{
  const system_property& prop = cxt.property;
  box_pos_ = local_pos() + vec2(prop.mergin);
  box_size_ = vec2((float)prop.font_size);
  rect name_area = cxt.font_renderer->get_area(prop.font_size, name());
  name_pos_ = local_pos() + vec2(box_size_.x + 2.f * prop.mergin, prop.mergin - name_area.y);
  set_size(box_pos_ + box_size_ - local_pos());

  return name_pos_ + vec2(prop.mergin + name_area.w, prop.mergin) - local_pos();
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

} // end of namespace gui
