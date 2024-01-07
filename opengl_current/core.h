#pragma once

#include <cstdint>
#include <string>
#include <string_view>  
#include <vector>

#include <filesystem>

#define ARRAY_NUM_ELEMENTS(Array) static_cast<std::int32_t>(sizeof(Array) / sizeof(Array[0]))
#define STD_ARRAY_NUM_ELEMENTS(Array) static_cast<std::int32_t>(Array.size())

template <typename CharType>
inline bool ContainsString(const std::basic_string<CharType>& str, const CharType* s) {
    std::size_t pos = str.find(s);
    return pos != std::basic_string<CharType>::npos;
}

std::vector<std::string> SplitString(const std::string& string, std::string_view delimiter);
std::string LoadFileContent(const std::filesystem::path& file_path);