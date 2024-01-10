#include "core.h"
#include <fstream>
#include <sstream>

std::vector<std::string> SplitString(const std::string& string, std::string_view delimiter)
{
    std::vector<std::string> tokens;
    std::size_t startOffset = 0;
    std::size_t delimiterPos = string.find(delimiter, startOffset);

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
    std::ifstream file(filePath.string());
    file.exceptions(std::ios::failbit | std::ios::badbit);
    std::ostringstream content;
    content << file.rdbuf();
    return content.str();
}
