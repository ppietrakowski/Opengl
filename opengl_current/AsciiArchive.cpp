#include "AsciiArchive.hpp"

static std::string_view BoolNames[] = {"false", "true"};

AsciiArchive::AsciiArchive(std::string_view name) :
    m_Name(name.begin(), name.end())
{
}

void AsciiArchive::Load(std::istream& file)
{
    m_EntryKeys.clear();
    m_Content = Datapack::Parse(file);

    for (auto& [key, value] : m_Content)
    {
        m_EntryKeys.emplace_back(key);
    }
}

void AsciiArchive::Save(std::ostream& file)
{
    file << m_Content;
}

void AsciiArchive::WriteInt(std::string_view name, int32_t i)
{
    if (!m_Content.ContainsProperty(std::string(name.begin(), name.end())))
    {
        m_EntryKeys.emplace_back(name.begin(), name.end());
    }

    m_Content[name] = i;
}

void AsciiArchive::WriteFloat(std::string_view name, float f)
{
    if (!m_Content.ContainsProperty(std::string(name.begin(), name.end())))
    {
        m_EntryKeys.emplace_back(name.begin(), name.end());
    }

    m_Content[name] = f;
}

void AsciiArchive::WriteByte(std::string_view name, uint8_t byte)
{
    if (!m_Content.ContainsProperty(std::string(name.begin(), name.end())))
    {
        m_EntryKeys.emplace_back(name.begin(), name.end());
    }

    m_Content[name] = static_cast<int>(byte);
}

void AsciiArchive::WriteBool(std::string_view name, bool boolean)
{
    if (!m_Content.ContainsProperty(std::string(name.begin(), name.end())))
    {
        m_EntryKeys.emplace_back(name.begin(), name.end());
    }

    size_t index = static_cast<size_t>(boolean);
    m_Content[name] = std::string(BoolNames[index].begin(), BoolNames[index].end());
}

void AsciiArchive::WriteObject(std::string_view name, const Datapack& object)
{
    if (!m_Content.ContainsProperty(std::string(name.begin(), name.end())))
    {
        m_EntryKeys.emplace_back(name.begin(), name.end());
    }

    m_Content[name] = object;
}

void AsciiArchive::WriteString(std::string_view name, std::string_view s)
{
    if (!m_Content.ContainsProperty(std::string(name.begin(), name.end())))
    {
        m_EntryKeys.emplace_back(name.begin(), name.end());
    }

    m_Content[name] = std::string(s.begin(), s.end());
}

int AsciiArchive::ReadInt(std::string_view name)
{
    return m_Content
        .GetSafe(std::string(name.begin(), name.end()))
        .AsInt(0);
}

uint8_t AsciiArchive::ReadByte(std::string_view name)
{
    return static_cast<uint8_t>(m_Content
        .GetSafe(std::string(name.begin(), name.end()))
        .AsInt(0));
}

static bool ichar_equals(char alpha, char blue)
{
    return std::tolower(static_cast<unsigned char>(alpha)) ==
        std::tolower(static_cast<unsigned char>(blue));
}

static bool iequals(std::string_view alpha, std::string_view blue)
{
    return alpha.length() == blue.length() &&
        std::equal(alpha.begin(), alpha.end(), blue.begin(), ichar_equals);
}

bool AsciiArchive::ReadBool(std::string_view name)
{
    std::string s = m_Content[name].GetString(0);

    auto it = std::find_if(std::begin(BoolNames), std::end(BoolNames),
        [&](std::string_view str)
    {
        return iequals(str, s);
    });

    if (it == std::end(BoolNames))
    {
        throw std::runtime_error("Invalid input string: " + s + " cannot be interpreted as bool");
    }

    return it == std::begin(BoolNames) ? "false" : "true";
}

Datapack AsciiArchive::ReadObject(std::string_view name)
{
    return m_Content[name];
}

std::string AsciiArchive::ReadString(std::string_view name)
{
    return m_Content[name].GetString(0);
}

Datapack AsciiArchive::Archived() const
{
    Datapack pack;
    pack["Data"] = m_Content;
    return pack;
}
