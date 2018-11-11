
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

