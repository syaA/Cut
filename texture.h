
#pragma once


class texture
{
public:
  typedef std::shared_ptr<texture> ptr_t;
  
public:

  texture();
  ~texture();

  GLuint texture_globj() { return texture_; }
  GLuint sampler_globj() { return sampler_; }

private:
  GLuint texture_;
  GLuint sampler_;


public:
  static auto make()
  {
    return std::make_shared<texture>();
  }
  static bool load_from_file(texture::ptr_t, const char *filename);
};

