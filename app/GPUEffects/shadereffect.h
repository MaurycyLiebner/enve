#ifndef SHADEREFFECT_H
#define SHADEREFFECT_H
#include "GPUEffects/gpurastereffect.h"

class ShaderEffectCaller : public GPURasterEffectCaller {
public:
    ShaderEffectCaller(const ShaderEffectProgram& program,
                       const UniformSpecifiers& uniformSpecifiers) :
        mProgram(program), mUniformSpecifiers(uniformSpecifiers) {}

    void render(QGL33c * const gl,
                GpuRenderTools& renderTools,
                GpuRenderData& data) {
        renderTools.requestTargetFbo().bind(gl);
        glClear(GL_COLOR_BUFFER_BIT);
        glActiveTexture(GL_TEXTURE0);
        renderTools.getSrcTexture().bind(gl);

        setupProgram(gl, data.fJSEngine, data.fPosX, data.fPosY);

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

    const ShaderEffectProgram mProgram;
    const UniformSpecifiers mUniformSpecifiers;
};

class ShaderEffect : public StaticComplexAnimator {
    friend class SelfRef;
    ShaderEffect(const ShaderEffectCreator * const creator,
                 const ShaderEffectProgram * const program,
                 const QString &name,
                 const QList<stdsptr<PropertyCreator>>& props);
public:
    virtual QMarginsF getMarginAtRelFrame(const qreal frame) {
        Q_UNUSED(frame);
        return QMarginsF();
    }

    void writeIdentifier(QIODevice * const dst) const;

    bool isVisible() const { return true; }

    stdsptr<GPURasterEffectCaller> getEffectCaller(const qreal relFrame) const {
        UniformSpecifiers uniformSpecifiers;
        const int argsCount = mProgram->fArgumentLocs.count();
        for(int i = 0; i < argsCount; i++) {
            const GLint& loc = mProgram->fArgumentLocs.at(i);
            const auto prop = ca_getChildAt(i);
            const auto& uniformC = mProgram->fUniformCreators.at(i);
            uniformSpecifiers << uniformC->create(loc, prop, relFrame);
        }
        return SPtrCreate(ShaderEffectCaller)(*mProgram, uniformSpecifiers);
    }

    GPUEffectAnimators *getParentEffectAnimators();

    void updateIfUsesProgram(const ShaderEffectProgram * const program) {
        if(program == mProgram)
            prp_afterWholeInfluenceRangeChanged();
    }
private:
    const ShaderEffectProgram * const mProgram;
    const ShaderEffectCreator * const mCreator;
};

#endif // SHADEREFFECT_H
