
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
    c->calc_layout(cxt);
    pos.y += c->size().y + prop.mergin;
    width = std::max(width, c->size().x);
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



void component_set::update()
{
  for (const auto& c : child_array_) {
    c->update();
  }
}

void component_set::draw(draw_context& cxt) const
{
  for (const auto& c : child_array_) {
    c->draw(cxt);
  }
}

void component_set::calc_layout(calc_layout_context& cxt)
{
  vec2 size = layout_vertical(local_pos(), cxt, child_array());
  set_size(size);
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
    shader_, texture_, vertex_buffer_, font_renderer_, screen_size_, property_ };

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

void window::calc_layout(calc_layout_context& cxt)
{
  const system_property& prop = cxt.property;
  rect name_area = cxt.font_renderer->get_area(prop.font_size, name());
  float width = name_area.w + prop.mergin * 2.f;
  auto pos = local_pos();
  name_pos_ = pos + vec2(prop.mergin, prop.mergin - name_area.y);
  pos.y += prop.mergin - name_area.y + prop.mergin;
  vec2 size = layout_vertical(pos, cxt, child_array());
  size.x = std::max(width, size.x);
  size.y = (pos.y + size.y) - local_pos().y;
  set_size(size);

  drag_.set_area(local_pos(), vec2(width, name_area.h + prop.mergin));
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
    in_press_(false), notice_variable_(notice), notice_function_(0)
{
}

button::button(const string& name, callback_t notice)
  : component(name), name_pos_(0.f), area_pos_(0.f), area_size_(0.f),
    in_press_(false), notice_variable_(0), notice_function_(notice)
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
  const color& col1 = in_press_ ? cxt.property.active_color : cxt.property.frame_color1;
  cxt.draw_rect(area_pos_, area_size_, cxt.property.frame_color0, col1);
  cxt.draw_font(name_pos_, cxt.property.font_color, name());
}

void button::calc_layout(calc_layout_context& cxt)
{
  const system_property& prop = cxt.property;
  rect name_area = cxt.font_renderer->get_area(prop.font_size, name());
  area_pos_ = local_pos() + vec2(prop.mergin, 0.f);
  area_size_.x = name_area.w + prop.mergin * 2.f;
  area_size_.y = name_area.h + prop.mergin * 2.f;
  name_pos_ = area_pos_ + vec2(prop.mergin, prop.mergin - name_area.y);

  set_size(area_size_ + vec2(prop.mergin, 0.f));
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

event_result button::on_cursor_leave(const vec2&)
{
  in_press_ = false;
  return { true, false };
}

} // end of namespace gui
