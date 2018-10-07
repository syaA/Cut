
#include "stdafx.h"

#include "gui.h"
#include "util.h"

namespace {

vertex_decl gui_vertex_decl[] = {
  { Semantics_Position, GL_FLOAT, 2, offsetof(gui::vertex, pos), sizeof(gui::vertex) },
  { Semantics_TexCoord_0, GL_FLOAT, 2, offsetof(gui::vertex, uv), sizeof(gui::vertex) },
};

} // end of namespace

namespace gui
{

void draw_context::draw(const vertex* vertex_array, int vertex_cnt,
                        const uint32_t *index_array, int index_cnt)
{
  gui_shader->use();

  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferData(GL_ARRAY_BUFFER, vertex_cnt * sizeof(vertex), vertex_array, GL_STATIC_DRAW);

  for (const auto& decl: gui_vertex_decl) {
    gui_shader->set_attrib(decl);
  }
  gui_shader->set_uniform("screen_size", screen_size);
  gui_shader->set_uniform("color0", property.frame_color0);
  gui_shader->set_uniform("color1", property.frame_color1);

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

void draw_context::draw_font(const vec2& pos, const string& str)
{
  font_renderer->render(pos, { property.font_size, property.font_size }, property.font_color, str);
}



component::component(const string& name)
  : name_(name), local_pos_(0.f, 0.f), size_(0.f, 0.f)
{
}

bool component::on_mouse_button(const vec2&, MouseButton, MouseAction, ModKey)
{
  return false;
}

bool component::on_cursor_move(const vec2&)
{
  return false;
}

bool component::on_cursor_enter(const vec2&)
{
  return false;
}

bool component::on_cursor_leave(const vec2&)
{
  return false;
}

bool component::on_mouse_scroll(const vec2&)
{
  return false;
}

bool component::on_input_key(int key, int scancode, KeyAction action, ModKey mod)
{
  return false;
}

bool component::on_input_char(char16_t code)
{
  return false;
}



void component_set::draw(draw_context& cxt) const
{
  for (const auto& c : child_array_) {
    c->draw(cxt);
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
    shader_(s), texture_(t), font_renderer_(f)
{
  glGenBuffers(1, &vertex_buffer_);

  // デフォルトプロパティ.
  property_.font_color = color(0.f, 0.f, 0.f, 1.f);
  property_.font_size = 16;
  property_.frame_color0 = color(0.f, 0.f, 0.f, 1.f);
  property_.frame_color1 = color(1.f, 1.f, 1.f, .7f);
  property_.mergin = 5.f;
}

system::~system()
{
  glDeleteBuffers(1, &vertex_buffer_);
}

void system::draw()
{
  glDisable(GL_DEPTH_TEST);

  font_renderer_->set_screen_size(screen_size_.x, screen_size_.y);

  draw_context cxt = {
    shader_, texture_, vertex_buffer_, font_renderer_, screen_size_, property_ };

  component_set::draw(cxt);
}

void system::calc_layout()
{
  calc_layout_context cxt = {
    font_renderer_, screen_size_, property_, {0.f, 0.f} };
  for (auto c : child_array()) {
    c->calc_layout(cxt);
  }
}



window::window(const string& name)
  : component_set(name)
{
}

void window::draw(draw_context& cxt) const
{
  auto x = local_pos().x;
  auto y = local_pos().y;
  auto w = size().x;
  auto h = size().y;
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
  const uint32_t index_array[] = {
    0, 1, 2, 2, 1, 3, 2, 3, 4, 4, 3, 5, 4, 5, 6, 6, 5, 7,
    1, 8, 3, 3, 8, 10, 3, 10, 5, 5, 10, 12, 5, 12, 7, 7, 12, 14,
    8, 9, 10, 10, 9, 11, 10, 11, 12, 12, 11, 13, 12, 13, 14, 14, 13, 15
  };
  cxt.draw(vertex_array, countof(vertex_array), index_array, countof(index_array));
  cxt.draw_font(local_pos() + name_pos_, name());

  component_set::draw(cxt);
}

void window::calc_layout(calc_layout_context& cxt)
{
  const system_property& prop = cxt.property;
  rect name_area = cxt.font_renderer->get_area(prop.font_size, name());
  float width = name_area.w + prop.mergin * 2.f;
  auto pos = local_pos();
  pos.x += prop.mergin;
  pos.y += -name_area.y + prop.mergin;
  name_pos_ = pos;
  vec2 prev = cxt.push_pos(pos);
  for (auto c : child_array()) {
    c->set_local_pos(pos);
    c->calc_layout(cxt);
    pos.y += c->size().y + prop.mergin;
    width = std::max(width, c->size().x);
  }
  cxt.pop_pos(prev);
  set_size({width, pos.y + prop.mergin});
}

} // end of namespace gui
