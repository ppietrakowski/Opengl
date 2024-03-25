#include "Datapack.hpp"
#include "Core.hpp"

#include <stack>
#include <iostream>

Datapack& Datapack::operator[](std::string_view name)
{
    std::string temp(name.begin(), name.end());
    auto it = m_NameToChildrenIndex.find(temp);

    if (it == m_NameToChildrenIndex.end())
    {
        size_t index = m_Children.size();
        m_NameToChildrenIndex[temp] = index;
        m_Children.emplace_back();
        m_Children.back().first = name;

        it = m_NameToChildrenIndex.find(temp);
    }

    return m_Children[it->second].second;
}

const Datapack& Datapack::GetSafe(const std::string& name) const
{
    size_t index = m_NameToChildrenIndex.at(name);
    return m_Children[index].second;
}

void Datapack::SetString(const std::string& value, size_t index)
{
    if (index >= m_Content.size())
    {
        m_Content.resize(index + 1);
    }

    m_Content[index] = value;
}

std::string Datapack::GetString(size_t index) const
{
    return m_Content.at(index);
}

int Datapack::AsInt(size_t index) const
{
    return std::stoi(m_Content.at(index));
}

double Datapack::AsNumber(size_t index) const
{
    return std::stod(m_Content.at(index));
}

float Datapack::AsFloatNumber(size_t index) const
{
    return std::stof(m_Content.at(index));
}

void Datapack::AppendString(const std::string& value)
{
    SetString(value, m_Content.size());
}

void Datapack::SetNumber(double value, size_t index)
{
    SetString(std::to_string(value), index);
}

void Datapack::AppendNumber(double value)
{
    SetNumber(value, m_Content.size());
}

void Datapack::SetNumber(int value, size_t index)
{
    SetString(std::to_string(value), index);
}

void Datapack::AppendNumber(int value)
{
    SetNumber(value, m_Content.size());
}

Datapack& Datapack::operator=(int number)
{
    Empty();
    SetNumber(number, 0);
    return *this;
}

Datapack& Datapack::operator=(double number)
{
    Empty();
    SetNumber(number, 0);
    return *this;
}

Datapack& Datapack::operator=(const std::string& str)
{
    Empty();
    SetString(str, 0);
    return *this;
}

IteratorContentDatapackAdapter<Datapack> Datapack::GetEachContentIterator() const
{
    return IteratorContentDatapackAdapter<Datapack>{*this, m_Content.size()};
}

std::vector<Datapack::DatapackPair>::const_iterator Datapack::begin() const
{
    return m_Children.begin();
}

std::vector<Datapack::DatapackPair>::const_iterator Datapack::end() const
{
    return m_Children.end();
}

size_t Datapack::GetNumContentValues() const
{
    return m_Content.size();
}

bool Datapack::IsContentOnly() const
{
    return !m_Content.empty();
}

bool Datapack::ContainsProperty(const std::string& str) const
{
    return m_NameToChildrenIndex.count(str) > 0;
}

Datapack Datapack::Parse(std::istream& stream)
{
    Datapack pack;
    std::stack<Datapack*> datapacks;

    std::string line;
    std::string lastName;

    while (std::getline(stream, line))
    {
        Trim(line);

        size_t equalSignPos = line.find("=");

        if (equalSignPos != std::string::npos)
        {
            std::string propName = line.substr(0, equalSignPos);
            std::string temp = line.substr(equalSignPos + 1);

            Trim(temp);
            Trim(propName);
            std::vector<std::string> content = std::move(SplitString(temp, ","));

            for (const std::string& s : content)
            {
                datapacks.top()->operator[](propName).AppendString(Trimmed(s));
            }
        }
        else
        {
            size_t bracketPos = line.find("{");

            if (bracketPos != std::string::npos)
            {
                if (datapacks.empty())
                {
                    datapacks.push(&pack);
                }
                else
                {
                    datapacks.push(&datapacks.top()->operator[](lastName));
                }

                continue;
            }
            else if (bracketPos = line.find("}"); bracketPos != std::string::npos)
            {
                datapacks.pop();
            }
            else
            {
                lastName = line;
            }
        }
    }

    return pack;
}

void Datapack::Empty()
{
    m_Content.clear();
    m_Children.clear();
    m_NameToChildrenIndex.clear();
}