
#include "stdafx.h"

#include <png.h>

#include "png_loader.h"

#include "util.h"

namespace {

void png_read_fstream(png_structp psp, png_bytep data, png_size_t len)
{
  std::ifstream *f = (std::ifstream*)png_get_io_ptr(psp);
  f->read((char*)data, len);
}

bool png_get_gl_format(int bit_depth, int color_type, GLint *internalformat, GLenum *format, GLenum *type)
{
  switch (bit_depth) {
  case 1:
  case 2:
  case 4:
    return false;
  case 8:
    *type = GL_UNSIGNED_BYTE;
    break;
  case 16:
    *type = GL_UNSIGNED_SHORT;
    break;
  }
  switch (color_type) {
  case PNG_COLOR_TYPE_GRAY:
    *internalformat = GL_RED;
    *format = GL_RED;
    break;
  case PNG_COLOR_TYPE_GRAY_ALPHA:
    *internalformat = GL_RG;
    *format = GL_RG;
    break;
  case PNG_COLOR_TYPE_RGB:
    *internalformat = GL_RGB;
    *format = GL_RGB;
    break;
  case PNG_COLOR_TYPE_RGB_ALPHA:
    *internalformat = GL_RGBA;
    *format = GL_RGBA;
    break;
  }

  return true;
}

} // end of anonymus namespace


bool load_png(texture *tex, const char *filename)
{
  // PNG 読み込み.
  std::ifstream f;
  f.open(filename, std::ios_base::binary);
  if (f.fail()) {
    return false;
  }
  png_structp psp = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  png_infop pip = png_create_info_struct(psp);
  png_set_read_fn(psp, &f, png_read_fstream);

  png_uint_32 width, height;
  int bit_depth, color_type, interlace_type;
  png_read_info(psp, pip);
  png_get_IHDR(psp, pip, &width, &height, &bit_depth, &color_type, &interlace_type, 0, 0);
  if (!is_power_of_2(width) || !is_power_of_2(height)) {
    png_destroy_read_struct(&psp, &pip, 0);
    return false;
  }

  GLint internalformat;
  GLenum format, type;
  if (!png_get_gl_format(bit_depth, color_type, &internalformat, &format, &type)) {
    png_destroy_read_struct(&psp, &pip, 0);
    return false;
  }

  auto rb = png_get_rowbytes(psp, pip);
  auto data = std::make_unique<uint8_t[]>(height * rb);
  std::vector<uint8_t*> row_array;
  row_array.reserve(height);
  for (png_uint_32 i=0; i<height; ++i) {
    row_array.push_back(&data[i * rb]);
  }
  png_read_image(psp, &row_array[0]);
  png_read_end(psp, pip);
  png_destroy_read_struct(&psp, &pip, 0);
  f.close();

  // テクスチャへロード.
  glBindTexture(GL_TEXTURE_2D, tex->texture_globj());
  glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, format, type, data.get());
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glGenerateMipmap(GL_TEXTURE_2D);

  return true;
}

