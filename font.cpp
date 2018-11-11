
#include "stdafx.h"

#include "font.h"


namespace font {

library_impl::library_impl()
{
  FT_Init_FreeType(&ft_library_);
}

library_impl::~library_impl()
{
  FT_Done_FreeType(ft_library_);
}


face::face(const char *file, int face_index)
  : face_index_(face_index)
{
  FT_New_Face(library::instance(), file, face_index, &ft_face_);
}

face::~face()
{
  FT_Done_Face(ft_face_);
}

FT_GlyphSlot face::get_glyph(char32_t c, int w, int h)
{
  FT_Set_Pixel_Sizes(ft_face_, w, h);
  FT_Load_Glyph(ft_face_, FT_Get_Char_Index(ft_face_, c), FT_LOAD_RENDER);
  return ft_face_->glyph;
}


texture_atlas::texture_atlas(int w, int h)
  : packer_(w, h)
{
  tex_ = texture::make();
  glBindTexture(GL_TEXTURE_2D, tex_->texture_globj());
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
  glSamplerParameteri(tex_->sampler_globj(), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glSamplerParameteri(tex_->sampler_globj(), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

rect texture_atlas::register_char(FT_GlyphSlot glyph)
{
  int width = glyph->bitmap.pitch;
  int height = (int)glyph->bitmap.rows;
  if ( width == 0) {
    width = 1;
  }
  if ( height == 0) {
    height = 1;
  }
  rect r = packer_.add({0.f, 0.f, (float)width, (float)height});
  if (r.empty()) {
    return r;
  }

  void * buf = glyph->bitmap.buffer;
  if (!buf) {
    static char dummy[] = {0};
    buf = dummy;
  }
  int pixel_store = 0;
  glGetIntegerv(GL_UNPACK_ALIGNMENT, &pixel_store);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glBindTexture(GL_TEXTURE_2D, tex_->texture_globj());
  glTexSubImage2D(GL_TEXTURE_2D, 0, (GLint)r.x, (GLint)r.y, (GLsizei)r.w, (GLsizei)r.h, GL_RED, GL_UNSIGNED_BYTE, buf);
  glPixelStorei(GL_UNPACK_ALIGNMENT, pixel_store);

  return r;
}

void texture_atlas::clear()
{
  packer_.clear();
}


renderer::renderer(face::ptr_t f, shader::ptr_t s)
  : face_(f), glyph_tex_(TEXTURE_SIZE, TEXTURE_SIZE), shader_(s)
{
  glGenBuffers(1, &vertex_buffer_);
}

renderer::~renderer()
{
  glDeleteBuffers(1, &vertex_buffer_);
}

size_t renderer::render(vec2 pos, const ivec2& size, const color& col, std::u32string_view str, vec2 *end)
{
  prepare_font(size, str);

  std::vector<vertex> vertex_array;
  vertex_array.reserve(str.size() * 2 + 2);
  static const float tex_size = (float)TEXTURE_SIZE;
  float bol = pos.x;
  for (auto c : str) {
    switch (c) {
    case u'\n':
      pos.y += face_->get_height();
      pos.x = bol;
      continue;
    case u'\r':
      pos.x = bol;
      continue;
    }
    auto info = glyph_table_[{c, size.x, size.y}];
    auto xy = vertex_from_screen({pos.x + info.bx, pos.y - info.by});
    auto wh = vec2{ info.uv.w / (screen_size_.x / 2.f), info.uv.h / (screen_size_.y / 2.f) };
    auto uv = vec2{ (info.uv.x + 0.5f) / tex_size , (info.uv.y + 0.5f) / tex_size };
    auto uvwh = vec2{ (info.uv.w - 1.f) / tex_size, (info.uv.h - 1.f) / tex_size };
    vertex v;
    v.col = col;
    v.pos = { xy.x, xy.y - wh.y };
    v.uv = { uv.x, uv.y + uvwh.y };
    vertex_array.push_back(v);
    v.pos = { xy.x, xy.y };
    v.uv = uv;
    vertex_array.push_back(v);
    v.pos = { xy.x + wh.x, xy.y - wh.y };
    v.uv = { uv.x + uvwh.x, uv.y + uvwh.y };
    vertex_array.push_back(v);
    v.pos = { xy.x + wh.x, xy.y - wh.y };
    v.uv = { uv.x + uvwh.x, uv.y + uvwh.y };
    vertex_array.push_back(v);
    v.pos = { xy.x, xy.y };
    v.uv = uv;
    vertex_array.push_back(v);
    v.pos = { xy.x + wh.x, xy.y };
    v.uv = { uv.x + uvwh.x, uv.y };
    vertex_array.push_back(v);
    pos.x += info.feed;
  }
  if (end) {
    *end = screen_from_vertex(pos);
  }

  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
  glBufferData(GL_ARRAY_BUFFER, vertex_array.size() * sizeof(vertex), vertex_array.data(), GL_STATIC_DRAW);

  shader_->use();
  static const vertex_decl vertex_decl[] = {
    { Semantics_Position, GL_FLOAT, 2, offsetof(vertex, pos), sizeof(vertex) },
    { Semantics_Color, GL_FLOAT, 4, offsetof(vertex, col), sizeof(vertex) },
    { Semantics_TexCoord_0, GL_FLOAT, 2, offsetof(vertex, uv), sizeof(vertex) },
  };
  for (auto& decl : vertex_decl) {
    shader_->set_attrib(decl);
  }
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, glyph_tex_.texture()->texture_globj());
  glBindSampler(0, glyph_tex_.texture()->sampler_globj());
  shader_->set_uniform("glyph_sampler", 0);

  glDrawArrays(GL_TRIANGLES, 0, (GLsizei)vertex_array.size());

  return vertex_array.size() / 6;
}

rect renderer::get_area(int size, std::u32string_view str, vec2 *end)
{
  return get_area({ size, size }, str, end);
}

rect renderer::get_area(const ivec2& size, std::u32string_view str, vec2 *end)
{
  prepare_font(size, str);

  vec2 lr{};
  vec2 rb{};
  vec2 pos{};
  for (auto c : str) {
    switch (c) {
    case u'\n':
      pos.y += face_->get_height();
      rb.y = std::max(pos.y, rb.y);
      pos.x = 0.f;
      continue;
    case u'\r':
      pos.x = 0.f;
      continue;
    }
    auto info = glyph_table_[{c, size.x, size.y}];
    lr.y = std::min(lr.y, pos.y - info.by);
    pos.x += info.feed;
    rb.x = std::max(pos.x, rb.x);
  }
  if (end) {
    *end = lr;
  }

  return { lr.x, lr.y, rb.x - lr.x, rb.y - lr.y};
}

void renderer::prepare_font(const ivec2& size, std::u32string_view str)
{
  // テクスチャを準備.
  // 途中で失敗したらクリアして一度だけやりなおす.
  bool tex_cleared = false;
  bool tex_done = false;
  while (!tex_done) {
    for (auto c : str) {
      auto info = glyph_table_[{c, size.x, size.y}];
      if (!info.uv.empty()) {
        continue;
      }
      auto glyph = face_->get_glyph(c, size.x, size.y);
      auto uv = glyph_tex_.register_char(glyph);
      if (!uv.empty()) {
        glyph_table_[{c, size.x, size.y}] =
          glyph_info{uv,
                     glyph->metrics.horiBearingX >> 6,
                     glyph->metrics.horiBearingY >> 6,
                     glyph->metrics.horiAdvance >> 6};
      } else {
        // 登録失敗.
        if (tex_cleared) {
          tex_done = true;
          break;
        }
        glyph_tex_.clear();
        glyph_table_.clear();
        tex_cleared = true;
        break;
      }
    }
    if (!tex_cleared) {
      tex_done = true;
    }
  }
}

vec2 renderer::vertex_from_screen(const vec2& v)
{
  return {v.x / screen_size_.x * 2.f - 1.f, v.y / screen_size_.y * -2.f + 1.f};
}
vec2 renderer::screen_from_vertex(const vec2& v)
{
  return {(v.x + 1.f) /2.f * screen_size_.x, (v.y - 1.f) / -2.f * screen_size_.x};
}


} // end of namespace font
