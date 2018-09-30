
#pragma once

#include "stdafx.h"

#include <filesystem>

std::string read_file_all(const char *filename)
{
  size_t size = std::filesystem::file_size(filename);
  
  std::ifstream f;
  f.open(filename);
  if (f.fail()) {
    return "";
  }

  std::string s(size, '\0');
  f.read(&s[0], size);
  f.close();

  return s;
}

template<class T>
void read(std::istream& in, T *p, int n)
{
  in.read((char*)p, sizeof(T) * n);
}

void read_uint8(std::istream& f, uint8_t *p, int n)
{
  read(f, p, n);
}
void read_int8(std::istream& f, int8_t *p, int n)
{
  read(f, p, n);
}
void read_uint16(std::istream& f, uint16_t *p, int n)
{
  read(f, p, n);
}
void read_int16(std::istream& f, int16_t *p, int n)
{
  read(f, p, n);
}
void read_uint32(std::istream& f, uint32_t *p, int n)
{
  read(f, p, n);
}
void read_int32(std::istream& f, int32_t *p, int n)
{
  read(f, p, n);
}
void read_float(std::istream& f, float *p, int n)
{
  read(f, p, n);
}
