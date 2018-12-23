#ifndef GPURASTEREFFECT_H
#define GPURASTEREFFECT_H
#include "Animators/complexanimator.h"
#include "Animators/qrealanimatorcreator.h"
#include "Animators/intanimatorcreator.h"
#include <QJSEngine>

class QDomElement;
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
        int val = ia->getCurrentIntEffectiveValueAtRelFrameF(relFrame);

        return [loc, val](QGL33c * const gl, QJSEngine&) {
            gl->glUniform1i(loc, val);
        };
    }
private:
    QString mScript;
};

typedef QList<stdsptr<UniformSpecifierCreator>> UniformSpecifierCreators;
struct GPURasterEffectProgram : public ShaderProgram {
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
    }
    static stdsptr<GPURasterEffectCreator> sLoadFromFile(
            QGL33c * const gl,
            const QString& filePath);
    static QList<stdsptr<GPURasterEffectCreator>> sEffectCreators;
};

class GPURasterEffectCaller : public StdSelfRef {
public:
    GPURasterEffectCaller(const GPURasterEffectProgram& program,
                          const UniformSpecifiers& uniformSpecifiers) :
        mProgram(program), mUniformSpecifiers(uniformSpecifiers) {

    }

    void setGlobalPos(const QPointF& gPos) {
        mGPosX = static_cast<GLfloat>(gPos.x());
        mGPosY = static_cast<GLfloat>(gPos.y());
    }

    void use(QGL33c * const gl, QJSEngine& engine) {
        gl->glUseProgram(mProgram.fID);
        Q_FOREACH(const auto& uni, mUniformSpecifiers) {
            uni(gl, engine);
        }
        if(mProgram.fGPosLoc >= 0) {
            gl->glUniform2f(mProgram.fGPosLoc, mGPosX, mGPosY);
        }
        gl->glUniform1i(mProgram.fTexLocation, 0);
    }
private:
    const GPURasterEffectProgram mProgram;
    GLfloat mGPosX;
    GLfloat mGPosY;
    const UniformSpecifiers mUniformSpecifiers;
};

class GPURasterEffect : public ComplexAnimator {
    friend class SelfRef;
public:
    virtual qreal getMargin() { return 0.; }
    virtual qreal getMarginAtRelFrame(const int &) { return 0.; }
    bool isVisible() { return true; }

    stdsptr<GPURasterEffectCaller> getGPURasterEffectCaller(
            const qreal& relFrame) {
        UniformSpecifiers uniformSpecifiers;
        int argsCount =  mProgram.fArgumentLocs.count();
        for(int i = 0; i < argsCount; i++) {
            const GLint& loc = mProgram.fArgumentLocs.at(i);
            Property* prop = ca_getChildAt(i);
            const auto& uniformC = mProgram.fUniformCreators.at(i);
            uniformSpecifiers << uniformC->create(loc, prop, relFrame);
        }
        return SPtrCreate(GPURasterEffectCaller)(mProgram, uniformSpecifiers);
    }
private:
    GPURasterEffectProgram mProgram;
    GPURasterEffect(const GPURasterEffectProgram &program,
                    const QString &name);
};

#endif // GPURASTEREFFECT_H
