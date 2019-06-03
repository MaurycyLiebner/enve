#ifndef GPURASTEREFFECT_H
#define GPURASTEREFFECT_H
#include "Animators/complexanimator.h"
#include "gpurastereffectcreator.h"
class GPUEffectAnimators;

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
        gl->glUseProgram(mProgram.fId);
        for(const auto& uni : mUniformSpecifiers) {
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
    GPURasterEffect(const GPURasterEffectProgram * const program,
                    const QString &name);
public:
    virtual qreal getMargin() { return 0; }
    virtual qreal getMarginAtRelFrame(const int &) { return 0; }
    bool isVisible() const { return true; }

    stdsptr<GPURasterEffectCaller> getGPURasterEffectCaller(
            const qreal& relFrame) {
        UniformSpecifiers uniformSpecifiers;
        const int argsCount = mProgram->fArgumentLocs.count();
        for(int i = 0; i < argsCount; i++) {
            const GLint& loc = mProgram->fArgumentLocs.at(i);
            const auto prop = ca_getChildAt(i);
            const auto& uniformC = mProgram->fUniformCreators.at(i);
            uniformSpecifiers << uniformC->create(loc, prop, relFrame);
        }
        return SPtrCreate(GPURasterEffectCaller)(*mProgram, uniformSpecifiers);
    }

    template <typename T = GPUEffectAnimators>
    T *getParentEffectAnimators() {
        return mParentEffects;
    }

    template <typename T = GPUEffectAnimators>
    void setParentEffectAnimators(T * const parentEffects) {
        mParentEffects = parentEffects;
    }

    void updateIfUsesProgram(const GPURasterEffectProgram * const program) {
        if(program == mProgram)
            prp_afterWholeInfluenceRangeChanged();
    }
private:
    const GPURasterEffectProgram * const mProgram;
    qptr<GPUEffectAnimators> mParentEffects;
};

#endif // GPURASTEREFFECT_H
