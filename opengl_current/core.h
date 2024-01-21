#pragma once

#include <cstdint>
#include <string>
#include <string_view>  
#include <vector>

#include <filesystem>

#define ARRAY_NUM_ELEMENTS(Array) static_cast<int>(sizeof(Array) / sizeof(Array[0]))
#define STD_ARRAY_NUM_ELEMENTS(Array) static_cast<int>(Array.size())

#ifdef _MSC_VER
#define FORCE_INLINE __forceinline
#else
##define FORCE_INLINE inline
#endif

template <typename CharType>
FORCE_INLINE bool ContainsString(const std::basic_string<CharType>& str, const CharType* s) {
    size_t pos = str.find(s);
    return pos != std::basic_string<CharType>::npos;
}

std::vector<std::string> SplitString(const std::string& string, std::string_view delimiter);
std::string LoadFileContent(const std::filesystem::path& file_path);

template <typename T>
void SafeDelete(T*& ptr) {
    delete ptr;
    ptr = nullptr;
}