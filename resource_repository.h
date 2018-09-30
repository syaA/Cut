
#pragma once

#include <any>

class resource_repository
{
public:
  typedef std::string key_t;
  typedef std::unordered_map<key_t, std::any> table_t;
  typedef std::shared_ptr<resource_repository> ptr_t;

public:
  resource_repository() {}

  template<class T>
  bool add(const key_t& name, const T& val)
  {
    if (table_.count(name)) {
      return false;
    }
    table_[name] = std::any(val);
    return true;
  }

  template<class T>
  T get(const key_t& name)
  {
    return std::any_cast<T>(table_[name]);
  }

private:
  table_t table_;
};
