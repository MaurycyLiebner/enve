#include "renderinstancesettings.h"
#include "canvas.h"
const std::map<int, QString> OutputSettings::SAMPLE_FORMATS_NAMES = {
    {AV_SAMPLE_FMT_U8, "8 bits unsigned"},
    {AV_SAMPLE_FMT_S16, "16 bits signed"},
    {AV_SAMPLE_FMT_S32, "32 bits signed"},
    {AV_SAMPLE_FMT_FLT, "32 bits float"},
    {AV_SAMPLE_FMT_DBL, "64 bits double"},
    {AV_SAMPLE_FMT_U8P, "8 bits unsigned, planar"},
    {AV_SAMPLE_FMT_S16P, "16 bits signed, planar"},
    {AV_SAMPLE_FMT_S32P, "32 bits signed, planar"},
    {AV_SAMPLE_FMT_FLTP, "32 bits float, planar"},
    {AV_SAMPLE_FMT_DBLP, "64 bits double, planar"},
    {AV_SAMPLE_FMT_S64, "64 bits signed"},
    {AV_SAMPLE_FMT_S64P, "64 bits signed, planar"}
};

QString OutputSettings::getChannelsLayoutNameStatic(const uint64_t &layout) {
    if(layout == AV_CH_LAYOUT_MONO) {
        return "Mono";
    } else if(layout == AV_CH_LAYOUT_STEREO) {
        return "Stereo";
    } else if(layout == AV_CH_LAYOUT_2POINT1) {
        return "2.1";
    } else if(layout == AV_CH_LAYOUT_SURROUND) {
        return "3.0";
    } else if(layout == AV_CH_LAYOUT_2_1) {
        return "3.0(back)";
    } else if(layout == AV_CH_LAYOUT_4POINT0) {
        return "4.0";
    } else if(layout == AV_CH_LAYOUT_QUAD) {
        return "Quad";
    } else if(layout == AV_CH_LAYOUT_2_2) {
        return "Quad(side)";
    } else if(layout == AV_CH_LAYOUT_3POINT1) {
        return "3.1";
    } else if(layout == AV_CH_LAYOUT_5POINT0_BACK) {
        return "5.0";
    } else if(layout == AV_CH_LAYOUT_5POINT0) {
        return "5.0(side)";
    } else if(layout == AV_CH_LAYOUT_4POINT1) {
        return "4.1";
    } else if(layout == AV_CH_LAYOUT_5POINT1_BACK) {
        return "5.1";
    } else if(layout == AV_CH_LAYOUT_5POINT1) {
        return "5.1(side)";
    } else if(layout == AV_CH_LAYOUT_6POINT0) {
        return "6.0";
    } else if(layout == AV_CH_LAYOUT_6POINT0_FRONT) {
        return "6.0(front)";
    } else if(layout == AV_CH_LAYOUT_HEXAGONAL) {
        return "Hexagonal";
    } else if(layout == AV_CH_LAYOUT_6POINT1) {
        return "6.1";
    } else if(layout == AV_CH_LAYOUT_6POINT1_BACK) {
        return "6.1(back)";
    } else if(layout == AV_CH_LAYOUT_6POINT1_FRONT) {
        return "6.1(front)";
    } else if(layout == AV_CH_LAYOUT_7POINT0) {
        return "7.0";
    } else if(layout == AV_CH_LAYOUT_7POINT0_FRONT) {
        return "7.0(front)";
    } else if(layout == AV_CH_LAYOUT_7POINT1) {
        return "7.1";
    } else if(layout == AV_CH_LAYOUT_7POINT1_WIDE_BACK) {
        return "7.1(wide)";
    } else if(layout == AV_CH_LAYOUT_7POINT1_WIDE) {
        return "7.1(wide-side)";
    } else if(layout == AV_CH_LAYOUT_OCTAGONAL) {
        return "Octagonal";
    } else if(layout == AV_CH_LAYOUT_HEXADECAGONAL) {
        return "Hexadecagonal";
    } else if(layout == AV_CH_LAYOUT_STEREO_DOWNMIX) {
        return "Downmix";
    } else if(layout == AV_CH_FRONT_LEFT) {
        return "Front left";
    } else if(layout == AV_CH_FRONT_RIGHT) {
        return "Front right";
    } else if(layout == AV_CH_FRONT_CENTER) {
        return "Front center";
    } else if(layout == AV_CH_LOW_FREQUENCY) {
        return "Low frequency";
    } else if(layout == AV_CH_BACK_LEFT) {
        return "Back left";
    } else if(layout == AV_CH_BACK_RIGHT) {
        return "Back right";
    } else if(layout == AV_CH_TOP_CENTER) {
        return "Top center";
    } else if(layout == AV_CH_TOP_FRONT_LEFT) {
        return "Top fron left";
    } else if(layout == AV_CH_TOP_FRONT_CENTER) {
        return "Top front center";
    } else if(layout == AV_CH_TOP_FRONT_RIGHT) {
        return "Top front right";
    } else if(layout == AV_CH_TOP_BACK_LEFT) {
        return "Top back left";
    } else if(layout == AV_CH_TOP_BACK_CENTER) {
        return "Top back center";
    } else if(layout == AV_CH_TOP_BACK_RIGHT) {
        return "Top back right";
    } else if(layout == AV_CH_STEREO_LEFT) {
        return "Stereo left";
    } else if(layout == AV_CH_STEREO_RIGHT) {
        return "Stereo right";
    } else if(layout == AV_CH_WIDE_LEFT) {
        return "Wide left";
    } else if(layout == AV_CH_WIDE_RIGHT) {
        return "Wide right";
    } else if(layout == AV_CH_SURROUND_DIRECT_LEFT) {
        return "Surround direct left";
    } else if(layout == AV_CH_SURROUND_DIRECT_RIGHT) {
        return "Surround direct right";
    } else if(layout == AV_CH_LOW_FREQUENCY_2) {
        return "Low frequency 2";
    } else if(layout == AV_CH_LAYOUT_NATIVE) {
        return "Native";
    }
    return "-";
}

RenderInstanceSettings::RenderInstanceSettings(Canvas* canvas) {
    setTargetCanvas(canvas);
    mRenderSettings.fMaxFrame = canvas->getMaxFrame();
}

const QString &RenderInstanceSettings::getName() {
    return mTargetCanvas->prp_getName();
}

void RenderInstanceSettings::setOutputDestination(
        const QString &outputDestination) {
    mOutputDestination = outputDestination;
}

const QString &RenderInstanceSettings::getOutputDestination() const {
    return mOutputDestination;
}

void RenderInstanceSettings::setTargetCanvas(Canvas *canvas) {
    mTargetCanvas = canvas;
}

Canvas *RenderInstanceSettings::getTargetCanvas() {
    return mTargetCanvas;
}

void RenderInstanceSettings::setCurrentRenderFrame(
        const int currentRenderFrame) {
    mCurrentRenderFrame = currentRenderFrame;
}

int RenderInstanceSettings::currentRenderFrame() {
    return mCurrentRenderFrame;
}

const OutputSettings &RenderInstanceSettings::getOutputRenderSettings() {
    return mOutputSettings;
}

void RenderInstanceSettings::setOutputRenderSettings(
        const OutputSettings &settings) {
    mOutputSettings = settings;
}

const RenderSettings &RenderInstanceSettings::getRenderSettings() {
    return mRenderSettings;
}

void RenderInstanceSettings::setRenderSettings(
        const RenderSettings &settings) {
    mRenderSettings = settings;
}

void RenderInstanceSettings::renderingAboutToStart() {
    copySettingsFromOutputSettingsProfile();
    mRenderError.clear();
    mRenderSettings.fFps = mTargetCanvas->getFps();
    mRenderSettings.fTimeBase = { 1, qRound(mRenderSettings.fFps) };
    mRenderSettings.fVideoWidth = mTargetCanvas->getCanvasWidth();
    mRenderSettings.fVideoHeight = mTargetCanvas->getCanvasHeight();
}
#include <QSound>
void RenderInstanceSettings::setCurrentState(
        const RenderInstanceSettings::RenderState &state,
        const QString &text) {
    mState = state;
    if(mState == ERROR) mRenderError = text;
    //if(mState == FINISHED) QSound::play(":/");
    emit stateChanged();
}

const QString &RenderInstanceSettings::getRenderError() const {
    return mRenderError;
}

RenderInstanceSettings::RenderState RenderInstanceSettings::getCurrentState() const {
    return mState;
}

void RenderInstanceSettings::copySettingsFromOutputSettingsProfile() {
    OutputSettingsProfile *profileT = mOutputSettingsProfile;
    if(!profileT) return;
    mOutputSettings = profileT->getSettings();
}

void RenderInstanceSettings::setOutputSettingsProfile(
        OutputSettingsProfile *profile) {
    if(!profile) {
        mOutputSettingsProfile.clear();
    } else {
        mOutputSettingsProfile = profile;
    }
    copySettingsFromOutputSettingsProfile();
}

OutputSettingsProfile *RenderInstanceSettings::getOutputSettingsProfile() {
    return mOutputSettingsProfile;
}
