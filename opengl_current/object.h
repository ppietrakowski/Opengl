#pragma once

#include <string>

class Object
{
public:
    Object() = default;
    virtual ~Object() = default;

public:
    virtual Object* Clone();

    virtual bool IsSameKindAs(const Object& object) const;
    virtual const char* ClassName() const;

    virtual void SetName(const std::string& name);

    const std::string& GetName() const
    {
        return m_Name;
    }

    virtual std::string ToString() const;

private:
    std::string m_Name;
};

#define CLASS_TYPE_TO_NAME(ClassType) #ClassType
#define DEFINE_CLASS_NOBASE() using Super = Object; virtual const char* ClassName() const override
#define IMPLEMENT_CLASS(ClassType) const char* ClassType :: ClassName() const { return CLASS_TYPE_TO_NAME(ClassType); }