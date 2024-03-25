#pragma once

#include <memory>
#include <string>

#include "Datapack.hpp"
#include "ClassRegistry.hpp"

#define DEFINE_CLASS_BASE(Class, Base) using Super = Base; static std::shared_ptr<Object> CreateNewInstance(std::string_view name); \
    Class(std::string_view name) : Super(name) {} \
    static std::shared_ptr<Class> CreateNewInstance_##Class(std::string_view name); \
    virtual const ClassDefinition& GetThisClassDefinition() const override; \
    static const ClassDefinition& GetClassDefinition(); \
    static std::string_view GetStaticClassName(); \
    virtual std::string_view GetThisClassName() const override;

#define IMPLEMENT_CLASS_BASE(Class, Base) std::shared_ptr<Object> Class::CreateNewInstance(std::string_view name) { return GetClassDefinition().createNewInstance(name); } \
    std::shared_ptr<Class> Class::CreateNewInstance_##Class(std::string_view name) { return std::dynamic_pointer_cast<Class>(GetClassDefinition().createNewInstance(name)); }\
    const ClassDefinition& Class::GetThisClassDefinition() const { return GetClassDefinition(); } \
    const ClassDefinition& Class::GetClassDefinition() { static ClassDefinition Definition{TYPE_TO_CLASS_NAME(Class), &Base::GetClassDefinition(), &_CreateInstanceObject<Class>}; return Definition; } \
    std::string_view Class::GetStaticClassName() { return TYPE_TO_CLASS_NAME(Class); } \
    std::string_view Class::GetThisClassName() const { return GetStaticClassName(); }

#define DEFINE_ABSTRACT_CLASS_BASE(Class, Base) using Super = Base; static std::shared_ptr<Object> CreateNewInstance(std::string_view name); \
    Class(std::string_view name) : Super(name) {} \
    virtual const ClassDefinition& GetThisClassDefinition() const override; \
    static const ClassDefinition& GetClassDefinition(); \
    static std::string_view GetStaticClassName();\
    virtual std::string_view GetThisClassName() const override;

#define IMPLEMENT_ABSTRACT_CLASS(Class, Base) std::shared_ptr<Object> Class::CreateNewInstance(std::string_view name) { return nullptr; } \
    const ClassDefinition& Class::GetThisClassDefinition() const { return GetClassDefinition(); } \
    const ClassDefinition& Class::GetClassDefinition() { static ClassDefinition Definition{TYPE_TO_CLASS_NAME(Class), &Base::GetClassDefinition(), nullptr}; return Definition; } \
    std::string_view Class::GetStaticClassName() { return TYPE_TO_CLASS_NAME(Class); } \
    std::string_view Class::GetThisClassName() const { return GetStaticClassName(); }

#define DEFINE_CLASS(Class) DEFINE_CLASS_BASE(Class, Object)
#define DEFINE_ABSTRACT_CLASS(Class) DEFINE_ABSTRACT_CLASS_BASE(Class, Object)

class Object
{
public:
    Object(std::string_view name);
    Object(const Object&) = delete;
    Object& operator=(const Object&) = delete;
    Object(Object&&) = delete;
    Object& operator=(Object&&) = delete;

    virtual ~Object() = default;

public:

    virtual std::shared_ptr<Object> Clone() const;

    std::string_view GetObjectName() const;
    std::string GetObjectNameStr() const;
    void SetObjectName(std::string_view newName);

    static std::shared_ptr<Object> CreateNewInstance(std::string_view name);

    template <typename T>
    bool IsSafeToCast() const
    {
        return !!dynamic_cast<const T*>(this);
    }

    template <typename T>
    std::shared_ptr<T> CastTo()
    {
        return std::dynamic_pointer_cast<T>(m_Self.lock());
    }

    template <typename T>
    std::shared_ptr<const T> CastTo() const
    {
        return std::dynamic_pointer_cast<const T>(m_Self.lock());
    }

    static const ClassDefinition& GetClassDefinition();
    virtual const ClassDefinition& GetThisClassDefinition() const;

    void Archive(Datapack& datapack);

    // Called when unarchiving object. This datapack will contain just properties of this object
    virtual void Unarchive(const Datapack& datapack);

    /* Prints class hierarchy from Object to this class */
    void DumpClassHierarchy(std::ostream& out) const;

    /* Prints class hierarchy from this class to Object etc. StaticMeshVisual -> Visual -> Object */
    void DumpClassHierarchyFromChild(std::ostream& out) const;

    template <typename T>
    void _Assign(std::shared_ptr<T> obj)
    {
        m_Self = obj;
    }

    static std::string_view GetStaticClassName();
    virtual std::string_view GetThisClassName() const;

protected:
    /* Should return all properties of this object that will be saved to global datapack */
    virtual Datapack Archived() const;

    std::weak_ptr<Object> m_Self;

private:
    std::string m_ObjectName;
};

template <typename T>
inline std::shared_ptr<Object> _CreateInstanceObject(std::string_view name)
{
    static_assert(!std::is_abstract_v<T>, "Cannot define _CreateInstanceObject for abstract objects");
    auto o = std::make_shared<T>(name);
    o->_Assign(o);
    return o;
}
