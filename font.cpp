
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

FT_GlyphSlot face::get_glyph(char16_t c, int w, int h)
{
  FT_Set_Pixel_Sizes(ft_face_, w, h);
  FT_Load_Glyph(ft_face_, FT_Get_Char_Index(ft_face_, c), FT_LOAD_RENDER);
  return ft_face_->glyph;
}


texture_atlas::texture_atlas(int w, int h)
  : packer_(w, h)
{
  // 真っ黒のテクスチャを生成しておく.
  auto data = std::make_unique<uint8_t[]>(w * h);
  std::memset(data.get(), 0x00, w * h);
  tex_ = texture::make();
  glBindTexture(GL_TEXTURE_2D, tex_->texture_globj());
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, data.get());
  glSamplerParameteri(tex_->sampler_globj(), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glSamplerParameteri(tex_->sampler_globj(), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

rect texture_atlas::register_char(FT_GlyphSlot glyph)
{
  rect r = packer_.add({0, 0, glyph->bitmap.pitch, (int)glyph->bitmap.rows});
  if (r.empty()) {
    return r;
  }

  int pixel_store = 0;
  glGetIntegerv(GL_UNPACK_ALIGNMENT, &pixel_store);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glBindTexture(GL_TEXTURE_2D, tex_->texture_globj());
  glTexSubImage2D(GL_TEXTURE_2D, 0, r.x, r.y, r.w, r.h, GL_RED, GL_UNSIGNED_BYTE, glyph->bitmap.buffer);
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

size_t renderer::render(vec2 pos, const ivec2& size, const color& col, std::u16string_view str, vec2 *end)
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
    auto xy = vec2((pos.x + info.bx) / screen_size_.x * 2.f - 1.f, (pos.y - info.by) / screen_size_.y * -2.f + 1.f);
    auto wh = vec2(info.uv.w / (screen_size_.x / 2.f), info.uv.h / (screen_size_.y / 2.f));
    auto uv = vec2((info.uv.x + 0.5f) / tex_size , (info.uv.y + 0.5f) / tex_size);
    auto uvwh = vec2((info.uv.w - 1.f)/ tex_size, (info.uv.h - 1.f) / tex_size);
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
    *end = pos;
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


} // end of namespace font
