#pragma once

#include <cstdint>
#include <string>

template <typename CharType>
inline bool ContainsString(const std::basic_string<CharType>& str, const CharType* s)
{
    std::size_t pos = str.find(s);
    return pos != std::basic_string<CharType>::npos;
}
