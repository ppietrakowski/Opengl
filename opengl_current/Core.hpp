#pragma once

#include <cstdint>
#include <string>
#include <string_view>  
#include <vector>
#include <unordered_map>
#include <map>
#include <span>

#include <filesystem>
#include <fstream>
#include <optional>

#define ARRAY_NUM_ELEMENTS(Array) static_cast<int>(sizeof(Array) / sizeof(Array[0]))

#ifdef _MSC_VER
#define FORCE_INLINE __forceinline
#else
##define FORCE_INLINE inline
#endif

template <typename CharType>
FORCE_INLINE bool ContainsString(const std::basic_string<CharType>& str, std::basic_string_view<CharType> s)
{
    size_t pos = str.find(s);
    return pos != std::basic_string<CharType>::npos;
}

std::vector<std::string> SplitString(std::string_view string, std::string_view delimiter);
std::string LoadFileContent(const std::filesystem::path& filePath);

template <typename T>
FORCE_INLINE void SafeDelete(T*& ptr)
{
    delete ptr;
    ptr = nullptr;
}

std::string FormatSize(size_t numBytes);

inline void Trim(std::string& s)
{
    s.erase(0, s.find_first_not_of("\t\r\n\f\v "));
    s.erase(s.find_last_not_of("\t\r\n\f\v ") + 1);
}

inline std::string Trimmed(const std::string& s)
{
    std::string str(s);
    Trim(str);
    return str;
}

template <class _Container>
inline int GetContainerSizeInt(const _Container& container)
{
    return static_cast<int>(std::ssize(container));
}

template <class T, size_t N>
inline int GetContainerSizeInt(const T(&)[N])
{
    return static_cast<int>(N);
}

template <typename T>
inline int GetTotalSizeOf(std::span<const T> values)
{
    return static_cast<int>(values.size_bytes());
}

template <typename T>
inline int GetTotalSizeOf(const std::vector<T>& values)
{
    return static_cast<int>(values.size() * sizeof(T));
}

#define FIND_MAP(map, key)  \
    auto it = map.find(key);\
                            \
    if (it == map.end())    \
    {                       \
        return nullptr;     \
    }                       \
                            \
    return &it->second;


template <typename Key, typename Value, typename Pred, typename Alloc, typename KeyLike>
inline Value* FindMap(std::map<Key, Value, Pred, Alloc>& map, const KeyLike& key)
{
    if constexpr (std::is_same_v<KeyLike, Key>)
    {
        FIND_MAP(map, key);
    }
    else
    {
        FIND_MAP(map, Key{key});
    }
}

template <typename Key, typename Value, typename Pred, typename Alloc, typename KeyLike>
inline const Value* FindMap(const std::map<Key, Value, Pred, Alloc>& map, const KeyLike& key)
{
    if constexpr (std::is_same_v<KeyLike, Key>)
    {
        FIND_MAP(map, key);
    }
    else
    {
        FIND_MAP(map, Key{key});
    }
}

template <typename Key, typename Value, typename Pred, typename Alloc, typename KeyLike>
inline Value* FindMap(std::unordered_map<Key, Value, Pred, Alloc>& map, const KeyLike& key)
{
    if constexpr (std::is_same_v<KeyLike, Key>)
    {
        FIND_MAP(map, key);
    }
    else
    {
        FIND_MAP(map, Key{key});
    }
}

template <typename Key, typename Value, typename Pred, typename Alloc, typename KeyLike>
inline const Value* FindMap(const std::unordered_map<Key, Value, Pred, Alloc>& map, const KeyLike& key)
{
    if constexpr (std::is_same_v<KeyLike, Key>)
    {
        FIND_MAP(map, key);
    }
    else
    {
        FIND_MAP(map, Key{key});
    }
}

#undef FIND_MAP

template <typename Container>
inline size_t LastIndex(const Container& container)
{
    size_t size = container.size();
    return size > 0 ? size - 1 : 0;
}

template <typename T, size_t N>
inline size_t LastIndex(const T(&)[N])
{
    return N > 0 ? N - 1 : 0;
}

template <typename Container>
inline bool IsValidIndex(const Container& container, int index)
{
    return index >= 0 && index < GetContainerSizeInt(container);
}

template <typename T, size_t N>
inline bool IsValidIndex(const T(&)[N], int index)
{
    return index >= 0 && index < static_cast<int>(N);
}

template <typename T>
void SerializeBinary(std::ofstream& file, const T& value)
{
    static_assert(std::is_standard_layout_v<T>);

    char buffer[sizeof(T)];

    memcpy(&buffer[0], &value, sizeof(T));
    file.write(&buffer[0], sizeof(T));
}

template <typename T>
T DeserializeBinary(std::ifstream& file)
{
    static_assert(std::is_standard_layout_v<T>);

    char buffer[sizeof(T)];

    file.read(&buffer[0], sizeof(T));
    T value;
    memcpy(&value, &buffer[0], sizeof(T));

    return value;
}
