#include "renderinstancesettings.h"
#include "canvas.h"
#include "renderinstancewidget.h"

RenderInstanceSettings::RenderInstanceSettings(Canvas *canvas) {
    setTargetCanvas(canvas);
    mRenderSettings.minFrame = 0;
    mRenderSettings.maxFrame = canvas->getMaxFrame();
}

const QString &RenderInstanceSettings::getName() {
    return mTargetCanvas->getName();
}

void RenderInstanceSettings::renderingAboutToStart() {
    copySettingsFromOutputSettingsProfile();
    mRenderError.clear();
    mRenderSettings.fps = mTargetCanvas->getFps();
    mRenderSettings.timeBase = { 1, qRound(mRenderSettings.fps) };
    mRenderSettings.videoWidth = mTargetCanvas->getCanvasWidth();
    mRenderSettings.videoHeight = mTargetCanvas->getCanvasHeight();
}

void RenderInstanceSettings::setCurrentState(
        const RenderInstanceSettings::RenderState &state,
        const QString &text) {
    mState = state;
    if(mState == ERROR) {
        mRenderError = text;
    }
    updateParentWidget();
}

void RenderInstanceSettings::updateParentWidget() {
    if(mParentWidget != NULL) {
        mParentWidget->updateFromSettings();
    }
}
