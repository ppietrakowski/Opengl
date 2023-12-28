#include "core.h"

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
