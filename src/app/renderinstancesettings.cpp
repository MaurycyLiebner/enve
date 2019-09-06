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
