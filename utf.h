
#pragma once

std::u16string u16_from_u8(std::string_view);
std::u32string u32_from_u8(std::string_view);

std::string u8_from_u32(std::u32string_view);
std::u16string u16_from_u32(std::u32string_view);

std::string u8_from_u16(std::u16string_view);
std::u32string u32_from_u16(std::u16string_view);

