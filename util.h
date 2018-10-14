
#pragma once


std::string read_file_all(const char *filename);

void read_uint8(std::istream&, uint8_t*, int n = 1);
void read_int8(std::istream&, int8_t*, int n = 1);
void read_uint16(std::istream&, uint16_t*, int n = 1);
void read_int16(std::istream&, int16_t*, int n = 1);
void read_uint32(std::istream&, uint32_t*, int n = 1);
void read_int32(std::istream&, int32_t*, int n = 1);
void read_float(std::istream&, float*, int n = 1);

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
