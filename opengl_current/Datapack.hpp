#pragma once

#include <string>
#include <vector>
#include <unordered_map>


template <typename DatapackType>
class IteratorContentDatapackAdapter
{
public:
    IteratorContentDatapackAdapter(const DatapackType& pack, size_t size) :
        m_Pack(&pack),
        m_Size(size)
    {
    }

    const std::string* begin() const
    {
        return m_Pack->GetString(0);
    }

    const std::string* end() const
    {
        return m_Pack->GetString(m_Size == 0 ? 0 : m_Size - 1);
    }

private:
    const DatapackType* m_Pack;
    size_t m_Size;
};

class Datapack
{
public:
    Datapack() = default;
    Datapack(const Datapack&) = default;
    Datapack(Datapack&&) noexcept = default;
    Datapack& operator=(Datapack&&) noexcept = default;
    Datapack& operator=(const Datapack&) = default;

public:
    using DatapackPair = std::pair<std::string, Datapack>;

    Datapack& operator[](std::string_view name);
    const Datapack& GetSafe(const std::string& name) const;

    void SetString(const std::string& value, size_t index);

    std::string GetString(size_t index) const;
    int AsInt(size_t index = 0) const;
    double AsNumber(size_t index = 0) const;
    float AsFloatNumber(size_t index = 0) const;

    void AppendString(const std::string& value);

    void SetNumber(double value, size_t index);
    void AppendNumber(double value);
    void SetNumber(int value, size_t index);
    void AppendNumber(int value);

    Datapack& operator=(int number);
    Datapack& operator=(double number);
    Datapack& operator=(const std::string& str);

    template <typename T>
    T& Dump(std::string_view firstPropertyName, T& stream) const
    {
        DumpImplementation(firstPropertyName, stream, 0);
        return stream;
    }

    IteratorContentDatapackAdapter<Datapack> GetEachContentIterator() const;

    std::vector<DatapackPair>::const_iterator begin() const;
    std::vector<DatapackPair>::const_iterator end() const;

    size_t GetNumContentValues() const;

    bool IsContentOnly() const;
    bool ContainsProperty(const std::string& str) const;

    static Datapack Parse(std::istream& stream);

    void Empty();

private:
    std::vector<std::string> m_Content;
    std::vector<DatapackPair> m_Children;
    std::unordered_map<std::string, size_t> m_NameToChildrenIndex;

private:
    template <typename T>
    static T& Indent(T& stream, size_t indentation)
    {
        for (size_t i = 0; i < indentation; ++i)
        {
            stream << "\t";
        }

        return stream;
    }

    template <typename T>
    void DumpImplementation(std::string_view firstPropertyName, T& stream, size_t indentation) const
    {
        Indent(stream, indentation) << firstPropertyName;

        if (!IsContentOnly())
        {
            stream << "\n";
            Indent(stream, indentation) << "{\n";

            for (auto&& [name, child] : m_Children)
            {
                child.DumpImplementation(name, stream, indentation + 1);
                stream << "\n";
            }

            Indent(stream, indentation) << "}";
        }
        else
        {
            stream << " = ";

            bool bFirst = true;

            for (const std::string& str : m_Content)
            {
                if (!bFirst)
                {
                    stream << ", ";
                }

                stream << str;
                bFirst = false;
            }
        }
    }

};

template <typename T>
inline T& operator<<(T& stream, const Datapack& pack)
{
    pack.Dump("", stream);
    return stream;
}