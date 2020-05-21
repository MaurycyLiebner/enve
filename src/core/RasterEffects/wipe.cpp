// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

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

#include "wipe.h"
#include "gpurendertools.h"

#include "Animators/qrealanimator.h"

WipeEffect::WipeEffect() :
    RasterEffect("Wipe", HardwareSupport::gpuPreffered,
                 true, RasterEffectType::WIPE) {
    mSharpness = enve::make_shared<QrealAnimator>(0, 0, 1, 0.01, "sharpness");
    ca_addChild(mSharpness);

    mDirection = enve::make_shared<QrealAnimator>(0, -9999, 9999, 1, "direction");
    ca_addChild(mDirection);

    mTime = enve::make_shared<QrealAnimator>(0, 0, 1, 0.01, "time");
    ca_addChild(mTime);
}

class WipeEffectCaller : public RasterEffectCaller {
public:
    WipeEffectCaller(const HardwareSupport hwSupport,
                     const qreal sharpness,
                     const qreal direction,
                     const qreal time) :
        RasterEffectCaller(hwSupport),
        mSharpness(sharpness),
        mDirection(direction),
        mTime(time) {}

    void processGpu(QGL33 * const gl,
                    GpuRenderTools& renderTools);

    void processCpu(CpuRenderTools& renderTools,
                    const CpuRenderData& data) {
        Q_UNUSED(renderTools)
        Q_UNUSED(data)
    }
private:
    static void sInitialize(QGL33 * const gl);

    static bool sInitialized;
    static GLuint sProgramId;

    static GLint sSharpnessU;
    static GLint sDirectionU;
    static GLint sTimeU;

    const qreal mSharpness;
    const qreal mDirection;
    const qreal mTime;
};

bool WipeEffectCaller::sInitialized = false;
GLuint WipeEffectCaller::sProgramId = 0;
GLint WipeEffectCaller::sSharpnessU = -1;
GLint WipeEffectCaller::sDirectionU = -1;
GLint WipeEffectCaller::sTimeU = -1;

stdsptr<RasterEffectCaller> WipeEffect::getEffectCaller(
        const qreal relFrame, const qreal resolution,
        const qreal influence, BoxRenderData * const data) const {
    Q_UNUSED(resolution)
    Q_UNUSED(data)

    const qreal sharpness = mSharpness->getEffectiveValue(relFrame);
    const qreal direction = mDirection->getEffectiveValue(relFrame);
    const qreal time = mTime->getEffectiveValue(relFrame) * influence;

    return enve::make_shared<WipeEffectCaller>(instanceHwSupport(),
                                               sharpness, direction, time);
}

void WipeEffectCaller::sInitialize(QGL33 * const gl) {
    try {
        iniProgram(gl, sProgramId, GL_TEXTURED_VERT, ":/shaders/wipe.frag");
    } catch(...) {
        RuntimeThrow("Could not initialize a program for WipeEffectCaller");
    }

    gl->glUseProgram(sProgramId);

    const auto texLocation = gl->glGetUniformLocation(sProgramId, "texture");
    gl->glUniform1i(texLocation, 0);

    sSharpnessU = gl->glGetUniformLocation(sProgramId, "sharpness");
    sDirectionU = gl->glGetUniformLocation(sProgramId, "direction");
    sTimeU = gl->glGetUniformLocation(sProgramId, "time");
}

void WipeEffectCaller::processGpu(QGL33 * const gl,
                                  GpuRenderTools &renderTools) {
    renderTools.switchToOpenGL(gl);

    if(!sInitialized) {
        sInitialize(gl);
        sInitialized = true;
    }

    renderTools.requestTargetFbo().bind(gl);
    gl->glClear(GL_COLOR_BUFFER_BIT);

    gl->glUseProgram(sProgramId);
    gl->glUniform1f(sSharpnessU, mSharpness);
    gl->glUniform1f(sDirectionU, mDirection);
    gl->glUniform1f(sTimeU, mTime);

    gl->glActiveTexture(GL_TEXTURE0);
    renderTools.getSrcTexture().bind(gl);

    gl->glBindVertexArray(renderTools.getSquareVAO());
    gl->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    renderTools.swapTextures();
}
