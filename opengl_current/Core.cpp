#include "Core.hpp"
#include <fstream>
#include <sstream>

#include <cassert>
#include <array>

#include "Logging.hpp"

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
    auto path = filePath.string();
    std::ifstream file(path.c_str());

    file.exceptions(std::ios::failbit | std::ios::badbit);
    ENG_LOG_VERBOSE("Loading file {}", path);

    std::ostringstream content;
    content << file.rdbuf();
    return content.str();
}

std::string FormatSize(size_t numBytes)
{
    static const char* Units[] =
    {
        "B", "KB", "MB", "GB", "TB", "PB"
    };

    float numUnits = static_cast<float>(numBytes);
    int64_t unitIndex = 0;

    while (numUnits > 1024)
    {
        ++unitIndex;
        numUnits /= 1024.0f;
    }

    return std::format("{:.2f} {}", numUnits, Units[unitIndex]);
}
