// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

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
