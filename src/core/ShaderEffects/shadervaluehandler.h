#ifndef SHADERVALUEHANDLER_H
#define SHADERVALUEHANDLER_H
#include <QJSEngine>

#include "glhelpers.h"
#include "smartPointers/ememory.h"

typedef std::function<void(QGL33 * const, QJSEngine&)> UniformSpecifier;

enum class GLValueType {
    Float, Vec2, Vec3, Vec4,
    Int, iVec2, iVec3, iVec4
};

class ShaderValueHandler : public StdSelfRef {
public:
    ShaderValueHandler(const QString& name,
                       const bool glValue,
                       const GLValueType type,
                       const QString& script);

    UniformSpecifier create(const GLint loc) const;

    const QString fName;
    const bool fGLValue;
private:
    const GLValueType mType;
    const QString mScript;
};

#endif // SHADERVALUEHANDLER_H
