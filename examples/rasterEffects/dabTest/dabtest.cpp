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

#include "dabtest.h"

qsptr<CustomRasterEffect> eCreateNewestVersion() {
    // Use default, most up to date, version
    return enve::make_shared<DabTest000>();
}

qsptr<CustomRasterEffect> eCreate(
        const CustomIdentifier &identifier) {
    Q_UNUSED(identifier)
    // Choose version based on identifier
    // if(identifier.fVersion == CustomIdentifier::Version{0, 0, 0})
    return enve::make_shared<DabTest000>();
}

// Returned value must be unique, lets enve distinguish effects
QString effectId() {
    return "3535afs7535gst";
}

// Name of your effect used in UI
QString eName() {
    return "Dab Test";
}

// here specify your effect's most up to date version
CustomIdentifier::Version effectVersion() {
    return { 0, 0, 0 };
}

CustomIdentifier eIdentifier() {
    return { effectId(), eName(), effectVersion() };
}

bool eSupports(const CustomIdentifier &identifier) {
    if(identifier.fEffectId != effectId()) return false;
    if(identifier.fEffectName != eName()) return false;
    return identifier.fVersion == effectVersion();
}

#include "enveCore/Animators/qrealanimator.h"

DabTest000::DabTest000() :
    CustomRasterEffect(eName().toLower(), HardwareSupport::gpuOnly, false) {
    mRadius = enve::make_shared<QrealAnimator>(0.5, 0, 0.5, 0.1, "radius");
    ca_addChild(mRadius);

    mHardness = enve::make_shared<QrealAnimator>(1, 0, 1, 0.1, "hardness");
    ca_addChild(mHardness);
}

stdsptr<RasterEffectCaller> DabTest000::getEffectCaller(
        const qreal relFrame, const qreal resolution, const qreal influence) const {
    const qreal radius = mRadius->getEffectiveValue(relFrame)*resolution*influence;
    const qreal hardness = mHardness->getEffectiveValue(relFrame);

    Dab dab;
    dab.fRadius = toSkScalar(radius);
    dab.fHardness = toSkScalar(hardness);
    dab.fSeg1Slope = -(1.f/dab.fHardness - 1.f);
    dab.fSeg2Offset = dab.fHardness/(1.f - dab.fHardness);
    dab.fSeg2Slope = -dab.fHardness/(1.f - dab.fHardness);
    return enve::make_shared<DabTestCaller000>(dab);
}

CustomIdentifier DabTest000::getIdentifier() const {
    return { effectId(), eName(), { 0, 0, 0 } };
}

bool DabTestCaller000::sInitialized = false;
GLuint DabTestCaller000::sDataId = 0;
GLuint DabTestCaller000::sProgramId = 0;

void DabTestCaller000::sInitialize(QGL33 * const gl) {
    try {
        iniProgram(gl, sProgramId, GL_TEXTURED_VERT,
                   ":/shaders/normal.frag");
    } catch(...) {
        RuntimeThrow("Could not initialize a program for DabTestCaller000");
    }

    gl->glGenTextures(1, &sDataId);

    gl->glBindTexture(GL_TEXTURE_2D, sDataId);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    gl->glUseProgram(sProgramId);

    const auto dabDataU = gl->glGetUniformLocation(sProgramId, "dabData");
    CheckInvalidLocation(dabDataU, "dabData");

    gl->glUniform1i(dabDataU, 0);
}

void DabTestCaller000::processGpu(QGL33 * const gl,
                              GpuRenderTools &renderTools,
                              GpuRenderData &data) {
    Q_UNUSED(data)
    renderTools.switchToOpenGL(gl);

    if(!sInitialized) {
        sInitialize(gl);
        sInitialized = true;
    }

    renderTools.requestTargetFbo();
    renderTools.swapTextures();
    renderTools.requestTargetFbo().bind(gl);

    gl->glUseProgram(sProgramId);

    gl->glActiveTexture(GL_TEXTURE0);
    gl->glBindTexture(GL_TEXTURE_2D, sDataId);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, 16, 1,
                     0, GL_RED, GL_FLOAT, &mDab);

    gl->glBindVertexArray(renderTools.getSquareVAO());
    gl->glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    renderTools.swapTextures();
}
