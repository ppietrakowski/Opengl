#include "object.h"
#include "logging.h"

Object* Object::Clone() {
    ELOG_WARNING(LOG_CORE, "Using default Object::Clone method");
    return new Object();
}

bool Object::IsSameKindAs(const Object& object) const {
    return true;
}

const char* Object::ClassName() const {
    return CLASS_TYPE_TO_NAME(Object);
}

void Object::SetName(const std::string& name) {
    name_ = name;
}

std::string Object::ToString() const {
    return std::string(CLASS_TYPE_TO_NAME(Object)) + "#" + std::to_string(reinterpret_cast<uintptr_t>(this));
}
