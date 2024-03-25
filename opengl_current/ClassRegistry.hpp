#pragma once

#include <string>
#include <memory>

#define TYPE_TO_CLASS_NAME(Type) (#Type)

inline constexpr int MaxClassNameLen = 256;

class Object;

struct ClassDefinition
{
    char className[MaxClassNameLen];
    const ClassDefinition* baseClassDefinition = nullptr;
    std::shared_ptr<Object>(*createNewInstance)(std::string_view name);

    std::shared_ptr<Object> MakeSharedInstance(std::string_view name) const;

    void PrintFromBase(std::ostream& os) const;
    void PrintFromChildren(std::ostream& os) const;
};

class IClassRegistry
{
public:
    virtual ~IClassRegistry() = default;
    virtual void RegisterObjectClass(std::string_view name, const ClassDefinition& definition) = 0;
    virtual const ClassDefinition& FindClassDefinition(std::string_view name) const = 0;
    virtual std::shared_ptr<Object> CreateObjectOfClass(std::string_view className, std::string_view objectName) const = 0;
};

IClassRegistry& GetClassRegistry();