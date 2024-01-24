#pragma once

#include <cstdint>
#include <string>
#include <string_view>  
#include <vector>

#include <filesystem>

#define ARRAY_NUM_ELEMENTS(Array) static_cast<int>(sizeof(Array) / sizeof(Array[0]))
#define STD_ARRAY_NUM_ELEMENTS(Array) static_cast<int>(Array.size())
#define STD_ARRAY_NUM_BYTES(Array, Type) static_cast<int>(Array.size() * sizeof(Type))

#ifdef _MSC_VER
#define FORCE_INLINE __forceinline
#else
##define FORCE_INLINE inline
#endif

#define ISIZE_OF(Object) static_cast<int>(sizeof(Object))

template <typename CharType>
FORCE_INLINE bool ContainsString(const std::basic_string<CharType>& str, const CharType* s)
{
    size_t pos = str.find(s);
    return pos != std::basic_string<CharType>::npos;
}

std::vector<std::string> SplitString(const std::string& string, std::string_view delimiter);
std::string LoadFileContent(const std::filesystem::path& filePath);

template <typename T>
FORCE_INLINE void SafeDelete(T*& ptr)
{
    delete ptr;
    ptr = nullptr;
}

std::string FormatSize(size_t numBytes);