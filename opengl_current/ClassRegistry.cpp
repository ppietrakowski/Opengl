#include "ClassRegistry.hpp"
#include "Object.hpp"
#include "ErrorMacros.hpp"

#include <unordered_map>
#include <iostream>
#include <cassert>


class ClassDefinitionHierarchyPrinter
{
public:
    ClassDefinitionHierarchyPrinter(std::ostream& out);

public:
    void PrintFromBase(const ClassDefinition& definition);
    void PrintFromChild(const ClassDefinition& definition);

    void ResetIndentationLevel();

private:
    int m_IndentationLevel = 0;
    std::ostream* m_OutStream = nullptr;
};

std::shared_ptr<Object> ClassDefinition::MakeSharedInstance(std::string_view name) const
{
    std::shared_ptr<Object> o = std::move(createNewInstance(name));
    o->_Assign(o);
    return o;
}

void ClassDefinition::PrintFromBase(std::ostream& os) const
{
    ClassDefinitionHierarchyPrinter printer(os);
    printer.PrintFromBase(*this);
}

void ClassDefinition::PrintFromChildren(std::ostream& os) const
{
    ClassDefinitionHierarchyPrinter printer(os);
    printer.PrintFromChild(*this);
}

ClassDefinitionHierarchyPrinter::ClassDefinitionHierarchyPrinter(std::ostream& out) :
    m_OutStream(&out)
{
}

void ClassDefinitionHierarchyPrinter::PrintFromBase(const ClassDefinition& definition)
{
    ASSERT(m_OutStream);

    if (definition.baseClassDefinition)
    {
        PrintFromBase(*definition.baseClassDefinition);
        m_IndentationLevel++;
    }

    for (int i = 0; i < 2 * m_IndentationLevel; ++i)
    {
        *m_OutStream << "-";
    }

    if (m_IndentationLevel != 0)
    {
        *m_OutStream << ">";
    }

    *m_OutStream << definition.className << std::endl;
}

void ClassDefinitionHierarchyPrinter::PrintFromChild(const ClassDefinition& definition)
{
    ASSERT(m_OutStream);

    for (int i = 0; i < 2 * m_IndentationLevel; ++i)
    {
        *m_OutStream << "-";
    }

    if (m_IndentationLevel != 0)
    {
        *m_OutStream << ">";
    }

    *m_OutStream << definition.className << std::endl;

    if (definition.baseClassDefinition)
    {
        m_IndentationLevel++;
        PrintFromChild(*definition.baseClassDefinition);
    }
}

void ClassDefinitionHierarchyPrinter::ResetIndentationLevel()
{
    m_IndentationLevel = 0;
}

class ClassRegistry : public IClassRegistry
{
public:
    virtual void RegisterObjectClass(std::string_view name, const ClassDefinition& definition) override
    {
        m_ClassNameToDefinition.try_emplace(std::string(name.begin(), name.end()), definition);
    }

    virtual const ClassDefinition& FindClassDefinition(std::string_view name) const override
    {
        return m_ClassNameToDefinition.at(std::string(name.begin(), name.end()));
    }

    virtual std::shared_ptr<Object> CreateObjectOfClass(std::string_view className, std::string_view objectName) const override
    {
        const ClassDefinition& definition = FindClassDefinition(className);
        return definition.createNewInstance(objectName);
    }

private:
    std::unordered_map<std::string, ClassDefinition> m_ClassNameToDefinition;
};

IClassRegistry& GetClassRegistry()
{
    static std::unique_ptr<IClassRegistry> registry;

    if (!registry)
    {
        registry = std::make_unique<ClassRegistry>();
    }

    return *registry;
}