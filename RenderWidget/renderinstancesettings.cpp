#include "renderinstancesettings.h"
#include "canvas.h"
#include "renderinstancewidget.h"

RenderInstanceSettings::RenderInstanceSettings() {
}

qreal RenderInstanceSettings::getFps() const {
    return mFps;
}

int RenderInstanceSettings::getVideoWidth() const {
    return mVideoWidth;
}

int RenderInstanceSettings::getVideoHeight() const {
    return mVideoHeight;
}

void RenderInstanceSettings::renderingAboutToStart() {
    mRenderError.clear();
    mFps = mTargetCanvas->getFps();
    mTimeBase = { 1, qRound(mFps) };
    mVideoWidth = mTargetCanvas->getCanvasWidth();
    mVideoHeight = mTargetCanvas->getCanvasHeight();
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
