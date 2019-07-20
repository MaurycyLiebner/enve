#include "dabtest.h"

qsptr<CustomGpuEffect> createNewestVersionEffect() {
    // Use default, most up to date, version
    return SPtrCreate(DabTest000)();
}

qsptr<CustomGpuEffect> createEffect(
        const CustomIdentifier &identifier) {
    Q_UNUSED(identifier);
    // Choose version based on identifier
    // if(identifier.fVersion == CustomIdentifier::Version{0, 0, 0})
    return SPtrCreate(DabTest000)();
}

// Returned value must be unique, lets enve distinguish effects
QString effectId() {
    return "3535afs7535gst";
}

// Name of your effect used in UI
QString effectName() {
    return "Dab Test";
}

// here specify your effect's most up to date version
CustomIdentifier::Version effectVersion() {
    return { 0, 0, 0 };
}

CustomIdentifier effectIdentifier() {
    return { effectId(), effectName(), effectVersion() };
}

bool supports(const CustomIdentifier &identifier) {
    if(identifier.fEffectId != effectId()) return false;
    if(identifier.fEffectName != effectName()) return false;
    return identifier.fVersion == effectVersion();
}

#include "Animators/qrealanimator.h"
DabTest000::DabTest000() :
    CustomGpuEffect(effectName().toLower()) {
    mRadius = SPtrCreate(QrealAnimator)(0.5, 0, 1, 0.1, "radius");
    ca_addChildAnimator(mRadius);

    mHardness = SPtrCreate(QrealAnimator)(1, 0, 1, 0.1, "hardness");
    ca_addChildAnimator(mHardness);
}

stdsptr<GPURasterEffectCaller>
        DabTest000::getEffectCaller(const qreal relFrame) const {
    const qreal radius = mRadius->getEffectiveValue(relFrame);
    const qreal hardness = mHardness->getEffectiveValue(relFrame);

    Dab dab;
    dab.fRadius = toSkScalar(radius);
    dab.fHardness = toSkScalar(hardness);
    dab.fSeg1Slope = -(1.f/dab.fHardness - 1.f);
    dab.fSeg2Offset = dab.fHardness/(1.f - dab.fHardness);
    dab.fSeg2Slope = -dab.fHardness/(1.f - dab.fHardness);
    return SPtrCreate(DabTestCaller000)(dab);
}

CustomIdentifier DabTest000::getIdentifier() const {
    return { effectId(), effectName(), { 0, 0, 0 } };
}

bool DabTestCaller000::sInitialized = false;
GLuint DabTestCaller000::sDataId = 0;
GLuint DabTestCaller000::sProgramId = 0;

void DabTestCaller000::sInitialize(QGL33 * const gl) {
    try {
        iniProgram(gl, sProgramId, GL_TEXTURED_VERT,
                   "/home/ailuropoda/Dev/enve/src/dabTest/normal.frag");
    } catch(...) {
        RuntimeThrow("Could not initialize a program for GPURasterEffect");
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

void DabTestCaller000::render(QGL33 * const gl,
                              GpuRenderTools &renderTools,
                              GpuRenderData &data) {
    Q_UNUSED(data);
    renderTools.switchToOpenGL();

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
}
