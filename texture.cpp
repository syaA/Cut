
#include "stdafx.h"

#include "texture.h"

#include "bmp_loader.h"
#include "png_loader.h"


texture::texture()
{
  glGenTextures(1, &texture_);
  glGenSamplers(1, &sampler_);
}

texture::~texture()
{
  glDeleteTextures(1, &texture_);
  glDeleteTextures(1, &sampler_);
}


bool texture::load_from_file(texture::ptr_t tex, const char *filename)
{
  std::filesystem::path path(filename);
  std::filesystem::path ext = path.extension();

  if (ext == ".bmp") {
    return load_bmp(tex.get(), filename);
  } else if (ext == ".png") {
    return load_png(tex.get(), filename);
  }

  return false;
}
