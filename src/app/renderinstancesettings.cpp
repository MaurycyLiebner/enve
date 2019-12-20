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

#include "renderinstancesettings.h"
#include "canvas.h"

RenderInstanceSettings::RenderInstanceSettings(Canvas* canvas) {
    setTargetCanvas(canvas);
}

RenderInstanceSettings::RenderInstanceSettings(const RenderInstanceSettings &src) :
    QObject() {
    setTargetCanvas(src.getTargetCanvas());
    mOutputDestination = src.mOutputDestination;
    mOutputSettingsProfile = src.mOutputSettingsProfile;
    mRenderSettings = src.mRenderSettings;
    mOutputSettings = src.mOutputSettings;
}

QString RenderInstanceSettings::getName() {
    return mTargetCanvas ? mTargetCanvas->prp_getName() : "-none-";
}

void RenderInstanceSettings::setOutputDestination(
        const QString &outputDestination) {
    mOutputDestination = outputDestination;
}

const QString &RenderInstanceSettings::getOutputDestination() const {
    return mOutputDestination;
}

void RenderInstanceSettings::setTargetCanvas(Canvas *canvas) {
    if(mTargetCanvas) disconnect(mTargetCanvas, nullptr, this, nullptr);
    if(canvas) {
        if(!mTargetCanvas) {
            mRenderSettings.fMaxFrame = canvas->getMaxFrame();
            mRenderSettings.fBaseWidth = canvas->getCanvasWidth();
            mRenderSettings.fBaseHeight = canvas->getCanvasHeight();
            mRenderSettings.fVideoWidth = qRound(mRenderSettings.fBaseWidth*
                                                 mRenderSettings.fResolution);
            mRenderSettings.fVideoHeight = qRound(mRenderSettings.fBaseHeight*
                                                  mRenderSettings.fResolution);
            mRenderSettings.fBaseFps = canvas->getFps();
            mRenderSettings.fFps = mRenderSettings.fBaseFps;
        }
        connect(canvas, &Canvas::dimensionsChanged,
                this, [this](const int width, const int height) {
            mRenderSettings.fBaseWidth = width;
            mRenderSettings.fBaseHeight = height;
            mRenderSettings.fVideoWidth = qRound(mRenderSettings.fBaseWidth*
                                                 mRenderSettings.fResolution);
            mRenderSettings.fVideoHeight = qRound(mRenderSettings.fBaseHeight*
                                                  mRenderSettings.fResolution);
        });
        connect(canvas, &Canvas::fpsChanged,
                this, [this](const qreal fps) {
            mRenderSettings.fBaseFps = fps;
            mRenderSettings.fFps = mRenderSettings.fBaseFps;
        });
    }
    mTargetCanvas = canvas;
}

Canvas *RenderInstanceSettings::getTargetCanvas() const {
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

const OutputSettings &RenderInstanceSettings::getOutputRenderSettings() const {
    return mOutputSettings;
}

void RenderInstanceSettings::setOutputRenderSettings(
        const OutputSettings &settings) {
    mOutputSettings = settings;
}

const RenderSettings &RenderInstanceSettings::getRenderSettings() const {
    return mRenderSettings;
}

void RenderInstanceSettings::setRenderSettings(
        const RenderSettings &settings) {
    mRenderSettings = settings;
}

void RenderInstanceSettings::renderingAboutToStart() {
    copySettingsFromOutputSettingsProfile();
    mRenderError.clear();
    mRenderSettings.fTimeBase = { 1, qRound(mRenderSettings.fFps) };
    mRenderSettings.fFrameInc = mRenderSettings.fBaseFps/mRenderSettings.fFps;
}

#include <QSound>
void RenderInstanceSettings::setCurrentState(const RenderState &state,
                                             const QString &text) {
    mState = state;
    if(mState == RenderState::error) mRenderError = text;
    //if(mState == FINISHED) QSound::play(":/");
    emit stateChanged(mState);
}

const QString &RenderInstanceSettings::getRenderError() const {
    return mRenderError;
}

RenderState RenderInstanceSettings::getCurrentState() const {
    return mState;
}

void RenderInstanceSettings::copySettingsFromOutputSettingsProfile() {
    if(!mOutputSettingsProfile) return;
    mOutputSettings = mOutputSettingsProfile->getSettings();
}

void RenderInstanceSettings::setOutputSettingsProfile(
        OutputSettingsProfile *profile) {
    if(!profile) mOutputSettingsProfile.clear();
    else mOutputSettingsProfile = profile;
    copySettingsFromOutputSettingsProfile();
}

OutputSettingsProfile *RenderInstanceSettings::getOutputSettingsProfile() {
    return mOutputSettingsProfile;
}
