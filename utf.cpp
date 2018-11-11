
#include "stdafx.h"

#include "utf.h"


std::u16string u16_from_u8(std::string_view)
{
  return u"";
}

std::u32string u32_from_u8(std::string_view)
{
  return U"";
}

std::string u8_from_u32(std::u32string_view s)
{
  std::string r;
  size_t len = s.size();
  for (size_t i=0; i<len; ++i) {
    const uint32_t c = s[i];
    if ((c >= 0x00 ) && (c <= 0x7f)) {
      r += c & 0b01111111;
    } else if ((c >= 0x0080 ) && (c <= 0x07ff)) {
      r += 0b11000000 | (c >> 6);
      r += 0b10000000 | (c & 0b00111111);
    } else if ((c >= 0x0800 ) && (c <= 0xffff)) {
      r += 0b11100000 | (c >> 12);
      r += 0b10000000 | ((c >> 6) & 0b00111111);
      r += 0b10000000 | (c & 0b00111111);
    } else if ((c >= 0x10000 ) && (c <= 0x1ffff)) {
      r += 0b11110000 | (c >> 18);
      r += 0b10000000 | ((c >> 12) & 0b00111111);
      r += 0b10000000 | ((c >> 6) & 0b00111111);
      r += 0b10000000 | (c & 0b00111111);
    } else {
      assert(!"invalid utf32 character.");
    }
  }

  return r;
}

std::u16string u16_from_u32(std::u32string_view)
{
  return u"";
}

std::string u8_from_u16(std::u16string_view s)
{
  return u8_from_u32(u32_from_u16(s));
}

std::u32string u32_from_u16(std::u16string_view s)
{
  std::u32string r;
  size_t len = s.size();
  for (size_t i=0; i<len; ++i) {
    const uint16_t c = s[i];
    if ((c & 0b1111110000000000) == 0b1101100000000000) {
      // surrogate pair
      assert(i < (len - 1));
      const uint16_t c1 = s[i + 1];
      assert((c1 & 0b1111110000000000) == 0b1101110000000000);
      r += (((c & 0b0000001111000000) + 1u) << 16) |
        ((c & 0b111111) << 10) |
          (c1 & 0b0000001111111111);
      ++i;
    } else {
      r += c;
    }
  }
  
  return r;
}
