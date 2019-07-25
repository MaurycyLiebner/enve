#ifndef SHADEREFFECT_H
#define SHADEREFFECT_H
#include "GPUEffects/gpueffect.h"
#include "shadereffectcreator.h"
#include "updatable.h"

class ShaderEffectCaller : public RasterEffectCaller {
public:
    ShaderEffectCaller(const QMargins& margin,
                       const ShaderEffectProgram& program,
                       const UniformSpecifiers& uniformSpecifiers) :
        RasterEffectCaller(false, margin), mProgram(program),
        mUniformSpecifiers(uniformSpecifiers) {}

    void processGpu(QGL33 * const gl,
                    GpuRenderTools& renderTools,
                    GpuRenderData& data) {
        renderTools.switchToOpenGL();

        renderTools.requestTargetFbo().bind(gl);
        gl->glClear(GL_COLOR_BUFFER_BIT);

        setupProgram(gl, data.fJSEngine, data.fPosX, data.fPosY);

        gl->glActiveTexture(GL_TEXTURE0);
        renderTools.getSrcTexture().bind(gl);

        gl->glBindVertexArray(renderTools.getSquareVAO());
        gl->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    HardwareSupport hardwareSupport() const {
        return HardwareSupport::GPU_ONLY;
    }

    void processCpu(CpuRenderTools& renderTools,
                    CpuRenderData& data) {
        Q_UNUSED(renderTools);
        Q_UNUSED(data);
    }
private:
    void setupProgram(QGL33 * const gl,
                      QJSEngine& engine,
                      const GLfloat gPosX,
                      const GLfloat gPosY) {
        gl->glUseProgram(mProgram.fId);
        for(const auto& uni : mUniformSpecifiers)
            uni(gl, engine);
        if(mProgram.fGPosLoc >= 0)
            gl->glUniform2f(mProgram.fGPosLoc, gPosX, gPosY);
    }

    const ShaderEffectProgram mProgram;
    const UniformSpecifiers mUniformSpecifiers;
};

class ShaderEffect : public GpuEffect {
    friend class SelfRef;
    ShaderEffect(const QString &name,
                 const ShaderEffectCreator * const creator,
                 const ShaderEffectProgram * const program,
                 const QList<stdsptr<PropertyCreator>>& props);
public:
    bool SWT_isShaderEffect() const { return true; }

    void writeIdentifier(QIODevice * const dst) const;

    bool isVisible() const { return true; }

    stdsptr<RasterEffectCaller> getEffectCaller(const qreal relFrame) const {
        UniformSpecifiers uniformSpecifiers;
        const int argsCount = mProgram->fArgumentLocs.count();
        for(int i = 0; i < argsCount; i++) {
            const GLint loc = mProgram->fArgumentLocs.at(i);
            const auto prop = ca_getChildAt(i);
            const auto& uniformC = mProgram->fUniformCreators.at(i);
            uniformSpecifiers << uniformC->create(loc, prop, relFrame);
        }
        const auto margin = getMarginAtRelFrame(relFrame);
        return SPtrCreate(ShaderEffectCaller)(margin, *mProgram, uniformSpecifiers);
    }

    void updateIfUsesProgram(const ShaderEffectProgram * const program) {
        if(program == mProgram)
            prp_afterWholeInfluenceRangeChanged();
    }
private:
    const ShaderEffectProgram * const mProgram;
    const ShaderEffectCreator * const mCreator;
};

#endif // SHADEREFFECT_H
