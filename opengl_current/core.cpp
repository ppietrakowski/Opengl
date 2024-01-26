#include "core.h"
#include <fstream>
#include <sstream>

#include <cassert>
#include <array>

#include "logging.h"

std::vector<std::string> SplitString(const std::string& string, std::string_view delimiter)
{
    std::vector<std::string> tokens;
    size_t startOffset = 0;
    size_t delimiterPos = string.find(delimiter, startOffset);

    while (delimiterPos != std::string::npos)
    {
        tokens.emplace_back(string.substr(startOffset, delimiterPos - startOffset));

        startOffset = delimiterPos + delimiter.length();
        delimiterPos = string.find(delimiter, startOffset);
    }

    tokens.emplace_back(string.substr(startOffset));
    return tokens;
}

std::string LoadFileContent(const std::filesystem::path& filePath)
{
    std::string path = filePath.string();

    if (!std::filesystem::exists(filePath))
    {
        ELOG_ERROR(LOG_CORE, "File %s not exists opening will fail", path.c_str());
    }

    std::ifstream file(path.c_str());

    ELOG_VERBOSE(LOG_CORE, "Loading file %s", path.c_str());

    file.exceptions(std::ios::failbit | std::ios::badbit);
    std::ostringstream content;
    content << file.rdbuf();
    return content.str();
}

std::string FormatSize(size_t numBytes)
{
    std::array<char, 100> strSize = {};
    static const char* Units[] =
    {
        "B", "KB", "MB", "GB", "TB", "PB"
    };

    float temp = static_cast<float>(numBytes);
    size_t unitIndex = 0;

    while (temp > 1024)
    {
        ++unitIndex;
        temp /= 1024.0f;
    }

    int lastIndex = sprintf(strSize.data(), "%.3f %s", temp, Units[unitIndex]);
    assert(lastIndex >= 0 && lastIndex < strSize.size());
    strSize.back() = 0;
    std::string str = strSize.data();
    return str;
}
