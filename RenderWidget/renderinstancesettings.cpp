#include "renderinstancesettings.h"
#include "canvas.h"

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

void RenderInstanceSettings::updateRenderVars() {
    mFps = mTargetCanvas->getFps();
    mTimeBase = { 1, qRound(mFps) };
    mVideoWidth = mTargetCanvas->getCanvasWidth();
    mVideoHeight = mTargetCanvas->getCanvasHeight();
}
