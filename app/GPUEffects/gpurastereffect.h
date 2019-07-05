#ifndef GPURASTEREFFECT_H
#define GPURASTEREFFECT_H
#include "Animators/staticcomplexanimator.h"
#include "gpurastereffectcreator.h"
class GPUEffectAnimators;

class GPURasterEffectCaller : public StdSelfRef {
public:
    GPURasterEffectCaller(const GPURasterEffectProgram& program,
                          const UniformSpecifiers& uniformSpecifiers) :
        mProgram(program), mUniformSpecifiers(uniformSpecifiers) {}

    void render(QGL33c * const gl,
                GpuRenderTools& renderTools,
                QJSEngine& engine,
                const GLfloat gPosX,
                const GLfloat gPosY) {
        renderTools.requestTargetFbo().bind(gl);
        glClear(GL_COLOR_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        renderTools.getSrcTexture().bind(gl);

        setupProgram(gl, engine, gPosX, gPosY);

        gl->glBindVertexArray(renderTools.getSquareVAO());
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
private:
    void setupProgram(QGL33c * const gl, QJSEngine& engine,
                      const GLfloat gPosX,
                      const GLfloat gPosY) {
        gl->glUseProgram(mProgram.fId);
        for(const auto& uni : mUniformSpecifiers)
            uni(gl, engine);
        if(mProgram.fGPosLoc >= 0)
            gl->glUniform2f(mProgram.fGPosLoc, gPosX, gPosY);
        gl->glUniform1i(mProgram.fTexLocation, 0);
    }

    const GPURasterEffectProgram mProgram;
    const UniformSpecifiers mUniformSpecifiers;
};

class GPURasterEffect : public StaticComplexAnimator {
    friend class SelfRef;
    GPURasterEffect(const GPURasterEffectCreator * const creator,
                    const GPURasterEffectProgram * const program,
                    const QString &name,
                    const QList<stdsptr<PropertyCreator>>& props);
public:
    virtual QMarginsF getMarginAtRelFrame(const qreal frame) {
        Q_UNUSED(frame);
        return QMarginsF();
    }

    void writeIdentifier(QIODevice * const dst) const;

    bool isVisible() const { return true; }

    stdsptr<GPURasterEffectCaller> getGPURasterEffectCaller(
            const qreal relFrame) {
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

    GPUEffectAnimators *getParentEffectAnimators();

    void updateIfUsesProgram(const GPURasterEffectProgram * const program) {
        if(program == mProgram)
            prp_afterWholeInfluenceRangeChanged();
    }
private:
    const GPURasterEffectProgram * const mProgram;
    const GPURasterEffectCreator * const mCreator;
};

#endif // GPURASTEREFFECT_H
