
#include "stdafx.h"

#include "bmp_loader.h"
#include "util.h"


namespace {

#pragma pack(push, 1)
struct bitmapfileheader
{
  uint16_t type;
  uint32_t filesize;
  uint32_t reserved;
  uint32_t offset;

  static const uint16_t Signature = 'B' | ('M' << 8);
};
struct bitmapfileinfo
{
  uint32_t size;
  int32_t width;
  int32_t height;
  uint16_t plane;
  uint16_t bit_count;
  uint32_t compression;
  uint32_t size_image;
  int32_t x_pix_per_meter;
  int32_t y_pix_per_meter;
  uint32_t clr_used;
  uint32_t clr_important;
};
#pragma pack(pop)

} // end of namespace

bool load_bmp(texture *tex, const char *filename)
{
  // BMP 読み込み.
  std::ifstream f;
  f.open(filename, std::ios_base::binary);
  if (f.fail()) {
    return false;
  }

  bitmapfileheader fh;
  f.read((char*)&fh, sizeof(fh));
  if (fh.type != bitmapfileheader::Signature) {
    return false;
  }
  bitmapfileinfo fi;
  f.read((char*)&fi, sizeof(fi));
  if (fi.size < sizeof(fi)) { // OS/2 ?
    return false;
  }
  if (fi.compression != 0) { // 圧縮非対応.
    return false;
  }
  if (fi.bit_count < 24) { // パレット非対応.
    return false;
  }
  if (!is_power_of_2(fi.width) || !is_power_of_2(fi.height)) {
    return false;
  }
  if ((fi.bit_count == 24) && (fi.width < 4)) { // パディング非対応.
    return false;
  }
  size_t size = fi.width * fi.height * fi.bit_count / 8;
  auto data = std::make_unique<uint8_t[]>(size);
  f.read((char*)data.get(), size);
  f.close();

  // テクスチャへロード.
  GLint internalformat = (fi.bit_count == 24) ? GL_RGB : GL_RGBA;
  GLenum format = (fi.bit_count == 24) ? GL_RGB : GL_RGBA;
  glBindTexture(GL_TEXTURE_2D, tex->texture_globj());
  glTexImage2D(GL_TEXTURE_2D, 0, internalformat, fi.width, fi.height, 0, format, GL_UNSIGNED_BYTE, data.get());
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glGenerateMipmap(GL_TEXTURE_2D);

  return true;
}



