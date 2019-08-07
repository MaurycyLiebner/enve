#ifndef UNIFORMSPECIFIERCREATOR_H
#define UNIFORMSPECIFIERCREATOR_H
#include "qrealanimatorcreator.h"
#include "intanimatorcreator.h"
#include <QJSEngine>

enum class ShaderPropertyType {
    qrealAnimator,
    intAnimator
};

typedef std::function<void(QGL33 * const, QJSEngine&)> UniformSpecifier;
typedef QList<UniformSpecifier> UniformSpecifiers;
struct UniformSpecifierCreator : public StdSelfRef {
    UniformSpecifierCreator(const ShaderPropertyType type,
                            const bool glValue) :
        mType(type), mGLValue(glValue) {}

    UniformSpecifier create(const GLint loc,
                            Property * const property,
                            const qreal relFrame) const;
private:
    const ShaderPropertyType mType;
    const bool mGLValue;
};

#endif // UNIFORMSPECIFIERCREATOR_H
