#include "renderinstancesettings.h"
#include "canvas.h"

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
    emit renderFrameChanged(currentRenderFrame);
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
    emit stateChanged(mState);
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
