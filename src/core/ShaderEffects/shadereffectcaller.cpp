#include "shadereffectcaller.h"

ShaderEffectCaller::ShaderEffectCaller(const QMargins &margin,
                                       const ShaderEffectProgram &program,
                                       const UniformSpecifiers &uniformSpecifiers) :
    RasterEffectCaller(HardwareSupport::gpuOnly, false, margin),
    mProgram(program), mUniformSpecifiers(uniformSpecifiers) {}

void ShaderEffectCaller::processGpu(QGL33 * const gl,
                                    GpuRenderTools &renderTools,
                                    GpuRenderData &data) {
    renderTools.switchToOpenGL(gl);

    renderTools.requestTargetFbo().bind(gl);
    gl->glClear(GL_COLOR_BUFFER_BIT);

    setupProgram(gl, data.fJSEngine, data.fPosX, data.fPosY);

    gl->glActiveTexture(GL_TEXTURE0);
    renderTools.getSrcTexture().bind(gl);

    gl->glBindVertexArray(renderTools.getSquareVAO());
    gl->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void ShaderEffectCaller::setupProgram(QGL33 * const gl, QJSEngine &engine,
                                      const GLfloat gPosX, const GLfloat gPosY) {
    gl->glUseProgram(mProgram.fId);
    for(const auto& uni : mUniformSpecifiers)
        uni(gl, engine);
    if(mProgram.fGPosLoc >= 0)
        gl->glUniform2f(mProgram.fGPosLoc, gPosX, gPosY);
}
