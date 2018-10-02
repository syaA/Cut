
#pragma once

template<class T>
class singleton
{
public:
  static T& instance()
  {
    static T inst;
    return inst;
  }

public:
  singleton(const singleton&) = delete;
  singleton& operator=(const singleton&) = delete;
  singleton(const singleton&&) = delete;
  singleton& operator=(const singleton&&) = delete;

private:
  singleton() = default;
  ~singleton() = default;
};

