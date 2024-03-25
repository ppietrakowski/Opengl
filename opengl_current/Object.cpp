#include "Object.hpp"

template <typename T>
std::shared_ptr<T> _CreateInstance(std::string_view name)
{
    static_assert(!std::is_abstract_v<T>, "Cannot define _CreateInstance for abstract objects");
    return std::make_shared<T>(name);
}

Object::Object(std::string_view name) :
    m_ObjectName(name.begin(), name.end())
{
}

std::shared_ptr<Object> Object::Clone() const
{
    std::string newName = m_ObjectName;
    newName += "01";

    return Object::CreateNewInstance(newName);
}

std::string_view Object::GetObjectName() const
{
    return m_ObjectName;
}

std::string Object::GetObjectNameStr() const
{
    return m_ObjectName;
}

void Object::SetObjectName(std::string_view newName)
{
    m_ObjectName.assign(newName.begin(), newName.end());
}

std::shared_ptr<Object> Object::CreateNewInstance(std::string_view name)
{
    std::shared_ptr<Object> sharedObj = std::move(_CreateInstance<Object>(name));
    sharedObj->m_Self = sharedObj;
    return sharedObj;
}

const ClassDefinition& Object::GetClassDefinition()
{
    static ClassDefinition Definition{TYPE_TO_CLASS_NAME(Object), nullptr, &_CreateInstance<Object>};
    return Definition;
}

const ClassDefinition& Object::GetThisClassDefinition() const
{
    return GetClassDefinition();
}

void Object::Archive(Datapack& datapack)
{
    Datapack& pack = datapack[m_ObjectName];
    pack = Archived();
}

void Object::Unarchive(const Datapack& datapack)
{
    m_ObjectName = datapack.GetSafe("ObjectName").GetString(0);
}

void Object::DumpClassHierarchy(std::ostream& out) const
{
    const ClassDefinition& definition = GetThisClassDefinition();
    definition.PrintFromBase(out);
}

void Object::DumpClassHierarchyFromChild(std::ostream& out) const
{
    const ClassDefinition& definition = GetThisClassDefinition();
    definition.PrintFromChildren(out);
}

std::string_view Object::GetStaticClassName()
{
    return TYPE_TO_CLASS_NAME(Object);
}

std::string_view Object::GetThisClassName() const
{
    return GetStaticClassName();
}

Datapack Object::Archived() const
{
    Datapack pack;
    pack["ObjectName"] = m_ObjectName;
    pack["ClassName"] = GetThisClassDefinition().className;

    return pack;
}