#ifndef GPURASTEREFFECTCREATOR_H
#define GPURASTEREFFECTCREATOR_H
#include "Animators/qrealanimatorcreator.h"
#include "Animators/intanimatorcreator.h"
#include <QJSEngine>

typedef std::function<void(QGL33c * const, QJSEngine&)> UniformSpecifier;
typedef QList<UniformSpecifier> UniformSpecifiers;
struct UniformSpecifierCreator : public StdSelfRef {
    virtual UniformSpecifier create(const GLint& loc,
                                    Property * const property,
                                    const qreal& relFrame) const = 0;
};

struct QrealAnimatorUniformSpecifierCreator :
        public UniformSpecifierCreator {
    QrealAnimatorUniformSpecifierCreator(const QString& script) :
        mScript(script) {}
    virtual UniformSpecifier create(const GLint& loc,
                                    Property * const property,
                                    const qreal& relFrame) const;
private:
    QString mScript;
};

struct IntAnimatorUniformSpecifierCreator :
        public UniformSpecifierCreator {
    IntAnimatorUniformSpecifierCreator(const QString& script) :
        mScript(script) {}
    virtual UniformSpecifier create(const GLint& loc,
                                    Property * const property,
                                    const qreal& relFrame) const {
        auto ia = GetAsPtr(property, IntAnimator);
        int val = ia->getCurrentIntEffectiveValueAtRelFrame(relFrame);

        return [loc, val](QGL33c * const gl, QJSEngine&) {
            gl->glUniform1i(loc, val);
        };
    }
private:
    QString mScript;
};

typedef QList<stdsptr<UniformSpecifierCreator>> UniformSpecifierCreators;
struct GPURasterEffectProgram {
    GLuint fId;
    QList<GLint> fArgumentLocs;
    GLint fGPosLoc;
    GLint fTexLocation;
    UniformSpecifierCreators fUniformCreators;
};

struct GPURasterEffectCreator : public PropertyCreator {
    GPURasterEffectCreator(const QString& name) : PropertyCreator(name) {}
    QList<stdsptr<PropertyCreator>> fProperties;
    GPURasterEffectProgram fProgram;

    qsptr<Property> create() const;
    virtual GLint getUniformLocation(
            QGL33c * const gl, const GLuint& program) const {
        Q_UNUSED(gl);
        Q_UNUSED(program);
        Q_ASSERT(false);
        return 0;
    }
    static stdsptr<GPURasterEffectCreator> sLoadFromFile(
            QGL33c * const gl,
            const QString& filePath);
    static QList<stdsptr<GPURasterEffectCreator>> sEffectCreators;
};

#endif // GPURASTEREFFECTCREATOR_H
