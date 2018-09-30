
#pragma once

class entity_world
{
public:
  typedef std::string key_t;
  typedef std::unordered_map<key_t, std::any> table_t;

public:
  entity_world() {}

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

  template<class T, class FuncT>
  void each(FuncT f)
  {
    for (auto & [key, a] : table_) {
      T o = std::any_cast<T>(a);
      if (o) {
        f(o);
      }
    }
  }

  template<class T, class FuncT>
  void each_with_name(FuncT f)
  {
    for (auto & [key, a] : table_) {
      T o = std::any_cast<T>(a);
      if (o) {
        f(o, key);
      }
    }
  }

private:
  table_t table_;
};

