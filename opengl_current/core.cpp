#include "core.h"
#include <fstream>
#include <sstream>

std::vector<std::string> SplitString(const std::string& string, std::string_view delimiter)
{
    std::vector<std::string> tokens;
    std::size_t start_offset = 0;
    std::size_t delimiter_pos = string.find(delimiter, start_offset);

    while (delimiter_pos != std::string::npos)
    {
        tokens.emplace_back(string.substr(start_offset, delimiter_pos - start_offset));

        start_offset = delimiter_pos + delimiter.length();
        delimiter_pos = string.find(delimiter, start_offset);
    }

    tokens.emplace_back(string.substr(start_offset));
    return tokens;
}

std::string LoadFileContent(const std::filesystem::path& file_path)
{
    std::ifstream file(file_path.string());
    file.exceptions(std::ios::failbit | std::ios::badbit);
    std::ostringstream content;
    content << file.rdbuf();
    return content.str();
}
