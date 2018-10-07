
#pragma once

#include "ft2build.h"
#include FT_FREETYPE_H

#include "singleton.h"
#include "rect_packer.h"
#include "texture.h"
#include "shader.h"


namespace font
{

class library_impl
{
public:
  library_impl();
  ~library_impl();

  operator FT_Library() { return ft_library_; }

private:
  FT_Library ft_library_;
};
typedef singleton<library_impl> library;


class face
{
public:
  typedef std::shared_ptr<face> ptr_t;

public:
  face(const char *file, int face_index);
  ~face();

  FT_GlyphSlot get_glyph(char16_t, int w, int h);

  int get_height() { return ft_face_->size->metrics.height >> 6; }

private:
  FT_Face ft_face_;
  int face_index_;
};


class texture_atlas
{
public:
  texture_atlas(int w, int h);

  rect register_char(FT_GlyphSlot);
  texture::ptr_t texture() { return tex_; }

  void clear();

private:
  rect_packer packer_;
  texture::ptr_t tex_;
};


struct vertex
{
  vec2 pos;
  color col;
  vec2 uv;
};

class renderer
{
public:
  static const int TEXTURE_SIZE = 1024;

public:
  typedef std::shared_ptr<renderer> ptr_t;

  typedef std::tuple<char16_t, int, int> glyph_key;
  struct glyph_key_hash {
    typedef std::size_t result_type;
    std::size_t operator()(const glyph_key& k) const
    {
      return std::hash<unsigned>()((std::get<0>(k) << 20) | (std::get<1>(k) << 10) | std::get<2>(k));
    }
  };
  struct glyph_info
  {
    rect uv;
    int bx;
    int by;
    int feed;
  };
  typedef std::unordered_map<glyph_key, glyph_info, glyph_key_hash> glyph_table_t;

public:
  renderer(face::ptr_t, shader::ptr_t);
  virtual ~renderer();

  size_t render(vec2 pos, const ivec2& size, const color&, std::u16string_view, vec2 *end = 0);
  rect get_area(const ivec2& size, std::u16string_view, vec2 *end=0);

  void prepare_font(const ivec2& size, std::u16string_view);

  void set_screen_size(int w, int h) { screen_size_ = { (float)w, (float)h }; }

protected:
  // 座標変換.
  vec2 vertex_from_screen(const vec2&);
  vec2 screen_from_vertex(const vec2&);

private:
  face::ptr_t face_;
  glyph_table_t glyph_table_;

  texture_atlas glyph_tex_;
  GLuint vertex_buffer_;
  shader::ptr_t shader_;

  vec2 screen_size_;
};

} // end of namespace font

