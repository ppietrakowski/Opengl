#pragma once
#pragma once

#include <vector>
#include <unordered_map>
#include <iostream>
#include <fstream>

#include "Core.hpp"
#include "Datapack.hpp"

struct BasicArchiveInfo
{
    std::string Name;
    int NumEntries = 0;
    bool bBinary = false;

    std::vector<std::string> EntryKeys;
};

class IArchive
{
public:
    virtual ~IArchive() = default;

    void Load(std::string_view path);
    virtual void Load(std::istream& file) = 0;

    void Save(std::string_view path);
    virtual void Save(std::ostream& file) = 0;

    virtual void WriteInt(std::string_view name, int32_t i) = 0;
    virtual void WriteFloat(std::string_view name, float f) = 0;
    virtual void WriteByte(std::string_view name, uint8_t byte) = 0;
    virtual void WriteBool(std::string_view name, bool boolean) = 0;
    virtual void WriteObject(std::string_view name, const Datapack& object) = 0;
    virtual void WriteString(std::string_view name, std::string_view s) = 0;

    virtual int ReadInt(std::string_view name) = 0;
    virtual uint8_t ReadByte(std::string_view name) = 0;
    virtual bool ReadBool(std::string_view name) = 0;
    virtual Datapack ReadObject(std::string_view name) = 0;
    virtual std::string ReadString(std::string_view name) = 0;

    virtual BasicArchiveInfo GetArchiveInfo() const = 0;
};

std::shared_ptr<IArchive> CreateEmptyAsciiArchive(std::string_view name = "archive");
