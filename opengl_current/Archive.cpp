#include "Archive.hpp"
#include "AsciiArchive.hpp"
#include "Core.hpp"

#include <fstream>

void IArchive::Load(std::string_view path)
{
    std::ifstream file(std::string(path.begin(), path.end()));
    Load(file);
}

void IArchive::Save(std::string_view path)
{
    std::ofstream file(std::string(path.begin(), path.end()));
    Save(file);
}

std::shared_ptr<IArchive> CreateEmptyAsciiArchive(std::string_view name)
{
    return std::make_shared<AsciiArchive>(name);
}
