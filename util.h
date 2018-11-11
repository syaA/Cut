
#pragma once


std::string read_file_all(const char *filename);


template<class PtrT>
void read(std::istream& in, PtrT p, int n=1)
{
  static_assert(std::is_pointer_v<PtrT>);
  in.read((char*)p, sizeof(*p) * n);
}

template<class T, int N>
void read(std::istream& in, T (&a)[N])
{
  return read(in, a, N);
}


#define countof(array) (sizeof(array)/sizeof(array[0]))

static const float PI = 3.14159265359f;

inline float deg2rad(float deg) { return deg / 180.f * PI; }
inline float raw2def(float rad) { return rad / PI * 180.f; }

template<class T>
inline T clamp(T x, T mn, T mx) { return x < mn ? mn : x > mx ? mx : x; }

inline bool is_power_of_2(int n) { return (n & (n - 1)) == 0; }
inline float roundup(float f) { return (float)(int)(f + 0.5f); }

template<class T>
class shared_ptr_creator
{
public:
  typedef std::shared_ptr<T> ptr_t;

private:
  struct holder_t : T
  {
    template<class... Args>
    holder_t(Args&&... args) : T(std::forward<Args>(args)...) {}
  };

public:
  template<class... Args>
  static ptr_t create(Args&&... args)
  {
    return std::make_shared<holder_t>(std::forward<Args>(args)...);
  }
};

template<class ContT>
class reverse_adaptor
{
public:
  typedef typename ContT::reverse_iterator iterator;
  typedef typename ContT::const_reverse_iterator const_iterator;

public:
  reverse_adaptor(ContT& cont)
    : cont_(cont) {}

  iterator begin() { return cont_.rbegin(); }
  iterator end() { return cont_.rend(); }
  const_iterator begin() const { return cont_.rbegin(); }
  const_iterator end() const { return cont_.rend(); }

private:
  ContT cont_;
};

template<class ContT>
reverse_adaptor<ContT> reverse(ContT& cont)
{
  return reverse_adaptor<ContT>(cont);
}

template<class T, class ContT>
class stack_for_range : public std::stack<T, ContT>
{
  using std::stack<T, ContT>::c;
public:
  auto begin() { return c.begin(); }
  auto end() { return c.end(); }
  auto begin() const { return c.begin(); }
  auto end() const { return c.end(); }
  auto rbegin() { return c.rbegin(); }
  auto rend() { return c.rend(); }
  auto rbegin() const { return c.rbegin(); }
  auto rend() const { return c.rend(); }
};


namespace std {
template<class T, class ContT>
auto begin(std::stack<T, ContT>& stk)
{
  return static_cast<stack_for_range<T, ContT>&>(stk).begin();
}
template<class T, class ContT>
auto end(std::stack<T, ContT>& stk)
{
  return static_cast<stack_for_range<T, ContT>&>(stk).end();
}
} // end of namespacce std
