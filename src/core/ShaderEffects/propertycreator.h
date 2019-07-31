#ifndef PROPERTYCREATOR_H
#define PROPERTYCREATOR_H
#include "Properties/property.h"
#include "smartPointers/stdselfref.h"
#include "glhelpers.h"

struct PropertyCreator : public StdSelfRef {
    PropertyCreator(const QString& name) : fName(name) {}

    QString fName;
    virtual qsptr<Property> create() const = 0;
    virtual GLint getUniformLocation(
            QGL33 * const gl, const GLuint& program) const = 0;
};

#endif // PROPERTYCREATOR_H
