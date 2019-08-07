#ifndef SHADERPROPERTYCREATOR_H
#define SHADERPROPERTYCREATOR_H
#include "Properties/property.h"
#include "smartPointers/stdselfref.h"
#include "glhelpers.h"

struct ShaderPropertyCreator : public StdSelfRef {
    ShaderPropertyCreator(const bool glValue, const QString& name) :
        fGLValue(glValue), fName(name) {}

    const bool fGLValue;
    const QString fName;

    virtual qsptr<Property> create() const = 0;
};

#endif // SHADERPROPERTYCREATOR_H
