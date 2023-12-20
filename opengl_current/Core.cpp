#include "Core.h"

std::vector<std::string> SplitString(const std::string& string, std::string_view delimiter)
{
    std::vector<std::string> tokens;
    std::size_t startOffset = 0;
    std::size_t posOfDelimiter = string.find(delimiter, startOffset);

    while (posOfDelimiter != std::string::npos)
    {
        tokens.push_back(string.substr(startOffset, posOfDelimiter - startOffset));

        startOffset = posOfDelimiter + delimiter.length();
        posOfDelimiter = string.find(delimiter, startOffset);
    }

    tokens.push_back(string.substr(startOffset));
    return tokens;
}
