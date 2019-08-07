#include "uniformspecifiercreator.h"

UniformSpecifier qrealAnimatorCreate(
        const bool glValue,
        const GLint loc,
        Property * const property,
        const qreal relFrame) {
    const auto qa = static_cast<QrealAnimator*>(property);
    const qreal val = qa->getEffectiveValue(relFrame);
    const QString propName = property->prp_getName();
    const QString valScript = propName + " = " + QString::number(val);

    if(glValue) {
        Q_ASSERT(loc >= 0);
        return [loc, val, valScript](QGL33 * const gl, QJSEngine& e) {
            e.evaluate(valScript);
            gl->glUniform1f(loc, static_cast<GLfloat>(val));
        };
    } else {
        return [valScript](QGL33 * const gl, QJSEngine& e) {
            Q_UNUSED(gl);
            e.evaluate(valScript);
        };
    }
}

UniformSpecifier intAnimatorCreate(
        const bool glValue,
        const GLint loc,
        Property * const property,
        const qreal relFrame) {
    const auto ia = static_cast<IntAnimator*>(property);
    const int val = ia->getEffectiveIntValue(relFrame);
    const QString propName = property->prp_getName();
    const QString valScript = propName + " = " + QString::number(val);

    if(glValue) {
        Q_ASSERT(loc >= 0);
        return [loc, val, valScript](QGL33 * const gl, QJSEngine& e) {
            e.evaluate(valScript);
            gl->glUniform1i(loc, val);
        };
    } else {
        return [valScript](QGL33 * const gl, QJSEngine& e) {
            Q_UNUSED(gl);
            e.evaluate(valScript);
        };
    }
}

UniformSpecifier UniformSpecifierCreator::create(const GLint loc,
                                                 Property * const property,
                                                 const qreal relFrame) const {
    if(mType == ShaderPropertyType::qrealAnimator)
        return qrealAnimatorCreate(mGLValue, loc, property, relFrame);
    else if(mType == ShaderPropertyType::intAnimator)
        return intAnimatorCreate(mGLValue, loc, property, relFrame);
    else RuntimeThrow("Unsupported type");
}
