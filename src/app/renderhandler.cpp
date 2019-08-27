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

#include "renderhandler.h"
#include "videoencoder.h"
#include "memoryhandler.h"
#include "Tasks/taskscheduler.h"
#include "canvas.h"
#include "Sound/soundcomposition.h"
#include "CacheHandlers/soundcachecontainer.h"
#include "CacheHandlers/sceneframecontainer.h"
#include "document.h"

RenderHandler* RenderHandler::sInstance = nullptr;

RenderHandler::RenderHandler(Document &document,
                             AudioHandler& audioHandler,
                             VideoEncoder& videoEncoder,
                             MemoryHandler& memoryHandler) :
    mDocument(document),
    mAudioHandler(audioHandler),
    mVideoEncoder(videoEncoder) {
    Q_ASSERT(!sInstance);
    sInstance = this;

    connect(&memoryHandler, &MemoryHandler::allMemoryUsed,
            this, &RenderHandler::outOfMemory);

    mPreviewFPSTimer = new QTimer(this);
    connect(mPreviewFPSTimer, &QTimer::timeout,
            this, &RenderHandler::nextPreviewFrame);
    connect(mPreviewFPSTimer, &QTimer::timeout,
            this, &RenderHandler::audioPushTimerExpired);

    const auto vidEmitter = videoEncoder.getEmitter();
//    connect(vidEmitter, &VideoEncoderEmitter::encodingStarted,
//            this, &SceneWindow::leaveOnlyInterruptionButtonsEnabled);
    connect(vidEmitter, &VideoEncoderEmitter::encodingFinished,
            this, &RenderHandler::interruptOutputRendering);
    connect(vidEmitter, &VideoEncoderEmitter::encodingInterrupted,
            this, &RenderHandler::interruptOutputRendering);
    connect(vidEmitter, &VideoEncoderEmitter::encodingFailed,
            this, &RenderHandler::interruptOutputRendering);
    connect(vidEmitter, &VideoEncoderEmitter::encodingStartFailed,
            this, &RenderHandler::interruptOutputRendering);
}

void RenderHandler::renderFromSettings(RenderInstanceSettings * const settings) {
    if(VideoEncoder::sStartEncoding(settings)) {
        setCurrentScene(settings->getTargetCanvas());
        mSavedCurrentFrame = mCurrentScene->getCurrentFrame();
        mSavedResolutionFraction = mCurrentScene->getResolutionFraction();

        mCurrentRenderSettings = settings;
        const RenderSettings &renderSettings = settings->getRenderSettings();
        setFrameAction(renderSettings.fMinFrame);

        const qreal resolutionFraction = renderSettings.fResolution;
        mMinRenderFrame = renderSettings.fMinFrame;
        mMaxRenderFrame = renderSettings.fMaxFrame;
        const qreal fps = mCurrentScene->getFps();
        mMaxSoundSec = qFloor(mMaxRenderFrame/fps);

        const auto nextFrameFunc = [this]() {
            nextSaveOutputFrame();
        };
        TaskScheduler::sSetFreeThreadsForCpuTasksAvailableFunc(nextFrameFunc);
        TaskScheduler::sSetAllTasksFinishedFunc(nextFrameFunc);

        //fitSceneToSize();
        if(!isZero6Dec(mSavedResolutionFraction - resolutionFraction)) {
            mCurrentScene->setResolutionFraction(resolutionFraction);
        }

        mCurrentRenderFrame = renderSettings.fMinFrame;
        mCurrRenderRange = {mCurrentRenderFrame, mCurrentRenderFrame};
        mCurrentScene->setCurrentRenderRange(mCurrRenderRange);

        mCurrentEncodeFrame = mCurrentRenderFrame;
        mFirstEncodeSoundSecond = qRound(mCurrentRenderFrame/fps);
        mCurrentEncodeSoundSecond = mFirstEncodeSoundSecond;
        if(!settings->getOutputRenderSettings().audioEnabled)
            mMaxSoundSec = mCurrentEncodeSoundSecond - 1;
        mCurrentSoundComposition->startBlockingAtFrame(mCurrentRenderFrame);
        mCurrentSoundComposition->scheduleFrameRange({mCurrentRenderFrame,
                                                      mCurrentRenderFrame});
        mCurrentScene->anim_setAbsFrame(mCurrentRenderFrame);
        mCurrentScene->setOutputRendering(true);
        TaskScheduler::sInstance->setAlwaysQue(true);
        nextCurrentRenderFrame();
        if(TaskScheduler::sAllQuedCpuTasksFinished()) {
            nextSaveOutputFrame();
        }
    }
}

void RenderHandler::setFrameAction(const int frame) {
    if(mCurrentScene) mCurrentScene->anim_setAbsFrame(frame);
    mDocument.actionFinished();
}

void RenderHandler::setCurrentScene(Canvas * const scene) {
    mCurrentScene = scene;
    mCurrentSoundComposition = scene ? scene->getSoundComposition() : nullptr;
}

void RenderHandler::nextCurrentRenderFrame() {
    auto& cacheHandler = mCurrentScene->getSceneFramesHandler();
    int newCurrentRenderFrame = cacheHandler.
            firstEmptyFrameAtOrAfter(mCurrentRenderFrame + 1);
    if(newCurrentRenderFrame - mCurrentRenderFrame > 1) {
        const int minBlock = mCurrentRenderFrame + 1;
        const int maxBlock = newCurrentRenderFrame - 1;
        cacheHandler.blockConts({minBlock, maxBlock}, true);
    }
    const bool allDone = newCurrentRenderFrame > mMaxRenderFrame;
    newCurrentRenderFrame = qMin(mMaxRenderFrame, newCurrentRenderFrame);
    const FrameRange newSoundRange = {mCurrentRenderFrame, newCurrentRenderFrame};
    mCurrentSoundComposition->scheduleFrameRange(newSoundRange);
    mCurrentSoundComposition->blockUpToFrame(newCurrentRenderFrame);

    mCurrentRenderFrame = newCurrentRenderFrame;
    mCurrRenderRange.fMax = mCurrentRenderFrame;
    mCurrentScene->setCurrentRenderRange(mCurrRenderRange);
    if(allDone) Document::sInstance->actionFinished();
    else setFrameAction(mCurrentRenderFrame);
}

void RenderHandler::renderPreview() {
    setCurrentScene(mDocument.fActiveScene);
    if(!mCurrentScene) return;
    const auto nextFrameFunc = [this]() {
        nextPreviewRenderFrame();
    };
    TaskScheduler::sSetFreeThreadsForCpuTasksAvailableFunc(nextFrameFunc);
    TaskScheduler::sSetAllTasksFinishedFunc(nextFrameFunc);

    mSavedCurrentFrame = mCurrentScene->getCurrentFrame();
    mCurrentRenderFrame = mSavedCurrentFrame;
    mCurrRenderRange = {mCurrentRenderFrame, mCurrentRenderFrame};
    mCurrentScene->setCurrentRenderRange(mCurrRenderRange);
    mCurrentSoundComposition->startBlockingAtFrame(mCurrentRenderFrame);

    mMaxRenderFrame = mCurrentScene->getMaxFrame();
    setRenderingPreview(true);

    emit previewBeingRendered();
    if(TaskScheduler::sAllQuedCpuTasksFinished()) {
        nextPreviewRenderFrame();
    }
}

void RenderHandler::interruptPreview() {
    if(mRenderingPreview) interruptPreviewRendering();
    else if(mPreviewing) stopPreview();
}

void RenderHandler::outOfMemory() {
    if(mRenderingPreview) {
        TaskScheduler::sClearTasks();
        playPreview();
    }
}

void RenderHandler::setRenderingPreview(const bool bT) {
    mRenderingPreview = bT;
    mCurrentScene->setRenderingPreview(bT);
    TaskScheduler::sInstance->setAlwaysQue(bT);
}

void RenderHandler::setPreviewing(const bool bT) {
    mPreviewing = bT;
    mCurrentScene->setPreviewing(bT);
    TaskScheduler::sInstance->setAlwaysQue(bT);
}

void RenderHandler::interruptPreviewRendering() {
    setRenderingPreview(false);
    TaskScheduler::sClearAllFinishedFuncs();
    clearPreview();
    auto& cacheHandler = mCurrentScene->getSceneFramesHandler();
    cacheHandler.blockConts({mSavedCurrentFrame + 1, mMaxRenderFrame}, false);
    setFrameAction(mSavedCurrentFrame);
    emit previewFinished();
}

void RenderHandler::interruptOutputRendering() {
    if(mCurrentScene) mCurrentScene->setOutputRendering(false);
    TaskScheduler::sInstance->setAlwaysQue(false);
    TaskScheduler::sClearAllFinishedFuncs();
    clearPreview();
    setFrameAction(mSavedCurrentFrame);
}

void RenderHandler::stopPreview() {
    setPreviewing(false);
    auto& cacheHandler = mCurrentScene->getSceneFramesHandler();
    cacheHandler.blockConts({mSavedCurrentFrame + 1, mMaxRenderFrame}, false);
    setFrameAction(mSavedCurrentFrame);
    mCurrentScene->setSceneFrame(mSavedCurrentFrame);
    mPreviewFPSTimer->stop();
    stopAudio();
    emit mCurrentScene->requestUpdate();
    emit previewFinished();
}

void RenderHandler::pausePreview() {
    if(mPreviewing) {
        mPreviewFPSTimer->stop();
        emit previewPaused();
    }
}

void RenderHandler::resumePreview() {
    if(mPreviewing) {
        mPreviewFPSTimer->start();
        emit previewBeingPlayed();
    }
}

void RenderHandler::playPreviewAfterAllTasksCompleted() {
    if(mRenderingPreview) {
        TaskScheduler::sSetFreeThreadsForCpuTasksAvailableFunc(nullptr);
        Document::sInstance->actionFinished();
        if(TaskScheduler::sAllTasksFinished()) {
            playPreview();
        } else {
            TaskScheduler::sSetAllTasksFinishedFunc([this]() {
                playPreview();
            });
        }
    }
}

void RenderHandler::playPreview() {
    if(!mCurrentScene) return;
    //setFrameAction(mSavedCurrentFrame);
    TaskScheduler::sClearAllFinishedFuncs();
    const int minPreviewFrame = mSavedCurrentFrame;
    const int maxPreviewFrame = qMin(mMaxRenderFrame, mCurrentRenderFrame);
    if(minPreviewFrame >= maxPreviewFrame) return;
    mMaxPreviewFrame = maxPreviewFrame;
    mCurrentPreviewFrame = minPreviewFrame;
    mCurrentScene->setSceneFrame(mCurrentPreviewFrame);
    mCurrentScene->setPreviewing(true);

    setRenderingPreview(false);
    setPreviewing(true);

    startAudio();
    const int mSecInterval = qRound(1000/mCurrentScene->getFps());
    mPreviewFPSTimer->setInterval(mSecInterval);
    mPreviewFPSTimer->start();
    emit previewBeingPlayed();
    emit mCurrentScene->requestUpdate();
}

void RenderHandler::nextPreviewRenderFrame() {
    if(!mRenderingPreview) return;
    if(mCurrentRenderFrame >= mMaxRenderFrame) {
        playPreviewAfterAllTasksCompleted();
    } else {
        nextCurrentRenderFrame();
        if(TaskScheduler::sAllTasksFinished()) {
            nextPreviewRenderFrame();
        }
    }
}

void RenderHandler::clearPreview() {
    emit previewFinished();
    stopPreview();
}

void RenderHandler::nextPreviewFrame() {
    if(!mCurrentScene) return;
    mCurrentPreviewFrame++;
    if(mCurrentPreviewFrame > mMaxPreviewFrame) {
        clearPreview();
    } else {
        mCurrentScene->setSceneFrame(mCurrentPreviewFrame);
        emit mCurrentScene->currentFrameChanged(mCurrentPreviewFrame);
    }
    emit mCurrentScene->requestUpdate();
}

void RenderHandler::finishEncoding() {
    TaskScheduler::sClearAllFinishedFuncs();
    mCurrentRenderSettings = nullptr;
    mCurrentScene->setOutputRendering(false);
    TaskScheduler::sInstance->setAlwaysQue(false);
    setFrameAction(mSavedCurrentFrame);
    if(!isZero4Dec(mSavedResolutionFraction - mCurrentScene->getResolutionFraction())) {
        mCurrentScene->setResolutionFraction(mSavedResolutionFraction);
    }
    mCurrentSoundComposition->unblockAll();
    VideoEncoder::sFinishEncoding();
}

void RenderHandler::nextSaveOutputFrame() {
    const auto& sCacheHandler = mCurrentSoundComposition->getCacheHandler();
    const qreal fps = mCurrentScene->getFps();
    const int sampleRate = eSoundSettings::sSampleRate();
    while(mCurrentEncodeSoundSecond <= mMaxSoundSec) {
        const auto cont = sCacheHandler.atFrame(mCurrentEncodeSoundSecond);
        if(!cont) break;
        const auto sCont = cont->ref<SoundCacheContainer>();
        const auto samples = sCont->getSamples();
        if(mCurrentEncodeSoundSecond == mFirstEncodeSoundSecond) {
            const int minSample = qRound(mMinRenderFrame*sampleRate/fps);
            const int max = samples->fSampleRange.fMax;
            VideoEncoder::sAddCacheContainerToEncoder(
                        samples->mid({minSample, max}));
        } else {
            VideoEncoder::sAddCacheContainerToEncoder(
                        enve::make_shared<Samples>(samples));
        }
        sCont->setInUse(false);
        mCurrentEncodeSoundSecond++;
    }
    if(mCurrentEncodeSoundSecond > mMaxSoundSec) VideoEncoder::sAllAudioProvided();

    const auto& cacheHandler = mCurrentScene->getSceneFramesHandler();
    while(mCurrentEncodeFrame <= mMaxRenderFrame) {
        const auto cont = cacheHandler.atFrame(mCurrentEncodeFrame);
        if(!cont) break;
        VideoEncoder::sAddCacheContainerToEncoder(cont->ref<SceneFrameContainer>());
        mCurrentEncodeFrame = cont->getRangeMax() + 1;
    }

    //mCurrentScene->renderCurrentFrameToOutput(*mCurrentRenderSettings);
    if(mCurrentRenderFrame >= mMaxRenderFrame) {
        if(mCurrentEncodeSoundSecond <= mMaxSoundSec) return;
        TaskScheduler::sSetFreeThreadsForCpuTasksAvailableFunc(nullptr);
        Document::sInstance->actionFinished();
        if(TaskScheduler::sAllTasksFinished()) {
            finishEncoding();
        } else {
            TaskScheduler::sSetAllTasksFinishedFunc([this]() {
                finishEncoding();
            });
        }
    } else {
        mCurrentRenderSettings->setCurrentRenderFrame(mCurrentRenderFrame);
        nextCurrentRenderFrame();
        if(TaskScheduler::sAllTasksFinished()) {
            nextSaveOutputFrame();
        }
    }
}

void RenderHandler::startAudio() {
    mAudioHandler.startAudio();
    mCurrentSoundComposition->start(mCurrentPreviewFrame);
}

void RenderHandler::stopAudio() {
    mAudioHandler.stopAudio();
    mCurrentSoundComposition->stop();
}

void RenderHandler::audioPushTimerExpired() {
    while(auto request = mAudioHandler.dataRequest()) {
        const qint64 len = mCurrentSoundComposition->read(
                    request.fData, request.fSize);
        if(len <= 0) break;
        request.fSize = int(len);
        mAudioHandler.provideData(request);
    }
}
