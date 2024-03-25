#pragma once

#include "Archive.hpp"

class AsciiArchive : public IArchive
{
public:
    AsciiArchive(std::string_view name);

    // Inherited via Archive
    virtual void Load(std::istream& file) override;
    virtual void Save(std::ostream& file) override;

    virtual void WriteInt(std::string_view name, int32_t i) override;
    virtual void WriteFloat(std::string_view name, float f) override;
    virtual void WriteByte(std::string_view name, uint8_t blue) override;

    virtual void WriteBool(std::string_view name, bool boolean) override;
    virtual void WriteObject(std::string_view name, const Datapack& object) override;
    virtual void WriteString(std::string_view name, std::string_view s) override;

    virtual int ReadInt(std::string_view name) override;
    virtual uint8_t ReadByte(std::string_view name) override;
    virtual bool ReadBool(std::string_view name) override;
    virtual Datapack ReadObject(std::string_view name) override;
    virtual std::string ReadString(std::string_view name) override;

    virtual BasicArchiveInfo GetArchiveInfo() const override
    {
        return BasicArchiveInfo{m_Name, static_cast<int>(m_Content.GetNumContentValues()), false, m_EntryKeys};
    }

    // can nest archives into itselfs
    Datapack Archived() const;

private:
    Datapack m_Content;

    // just to fast return get archive info
    std::vector<std::string> m_EntryKeys;
    std::string m_Name;
};

