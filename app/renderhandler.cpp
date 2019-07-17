#include "renderhandler.h"
#include "videoencoder.h"
#include "memoryhandler.h"
#include "taskscheduler.h"
#include "canvas.h"
#include "Sound/soundcomposition.h"
#include "CacheHandlers/soundcachecontainer.h"
#include "document.h"

RenderHandler::RenderHandler(Document &document,
                             AudioHandler& audioHandler) :
    mDocument(document), mAudioHandler(audioHandler) {
    connect(MemoryHandler::sGetInstance(), &MemoryHandler::allMemoryUsed,
            this, &RenderHandler::outOfMemory);

    mPreviewFPSTimer = new QTimer(this);
    connect(mPreviewFPSTimer, &QTimer::timeout,
            this, &RenderHandler::nextPreviewFrame);
    connect(mPreviewFPSTimer, &QTimer::timeout,
            this, &RenderHandler::audioPushTimerExpired);

    const auto vidEmitter = VideoEncoder::getVideoEncoderEmitter();
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
    VideoEncoder::sStartEncoding(settings);
    if(VideoEncoder::sEncodingSuccessfulyStarted()) {
        mSavedCurrentFrame = mCurrentScene->getCurrentFrame();
        mSavedResolutionFraction = mCurrentScene->getResolutionFraction();

        mCurrentRenderSettings = settings;
        const RenderSettings &renderSettings = settings->getRenderSettings();
        setCurrentScene(settings->getTargetCanvas());
        setFrameAction(renderSettings.fMinFrame);

        const qreal resolutionFraction = renderSettings.fResolution;
        mMinRenderFrame = renderSettings.fMinFrame;
        mMaxRenderFrame = renderSettings.fMaxFrame;

        const auto nextFrameFunc = [this]() {
            nextSaveOutputFrame();
        };
        TaskScheduler::sSetFreeThreadsForCPUTasksAvailableFunc(nextFrameFunc);
        TaskScheduler::sSetAllTasksFinishedFunc(nextFrameFunc);

        //fitSceneToSize();
        if(!isZero6Dec(mSavedResolutionFraction - resolutionFraction)) {
            mCurrentScene->setResolutionFraction(resolutionFraction);
        }

        mCurrentRenderFrame = renderSettings.fMinFrame;
        mCurrRenderRange = {mCurrentRenderFrame, mCurrentRenderFrame};
        mCurrentScene->setCurrentRenderRange(mCurrRenderRange);

        mCurrentEncodeFrame = mCurrentRenderFrame;
        mFirstEncodeSoundSecond = qRound(mCurrentRenderFrame/mCurrentScene->getFps());
        mCurrentEncodeSoundSecond = mFirstEncodeSoundSecond;
        mCurrentSoundComposition->startBlockingAtFrame(mCurrentRenderFrame);
        mCurrentSoundComposition->scheduleFrameRange({mCurrentRenderFrame,
                                                      mCurrentRenderFrame});
        mCurrentScene->anim_setAbsFrame(mCurrentRenderFrame);
        mCurrentScene->setOutputRendering(true);
        if(TaskScheduler::sAllQuedCPUTasksFinished()) {
            nextSaveOutputFrame();
        }
    }
}

void RenderHandler::setFrameAction(const int frame) {
    if(mCurrentScene) mCurrentScene->anim_setAbsFrame(frame);
    emit queTasksAndUpdate();
}

void RenderHandler::setCurrentScene(Canvas * const scene) {
    mCurrentScene = scene;
    mCurrentSoundComposition = scene ?
                scene->getSoundComposition() : nullptr;
}

void RenderHandler::nextCurrentRenderFrame() {
    auto& cacheHandler = mCurrentScene->getCacheHandler();
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
    if(!allDone) setFrameAction(mCurrentRenderFrame);
}

void RenderHandler::renderPreview() {
    setCurrentScene(mDocument.fActiveScene);
    if(!mCurrentScene) return;
    const auto nextFrameFunc = [this]() {
        nextPreviewRenderFrame();
    };
    TaskScheduler::sSetFreeThreadsForCPUTasksAvailableFunc(nextFrameFunc);
    TaskScheduler::sSetAllTasksFinishedFunc(nextFrameFunc);

    mSavedCurrentFrame = mCurrentScene->getCurrentFrame();
    mCurrentRenderFrame = mSavedCurrentFrame;
    mCurrRenderRange = {mCurrentRenderFrame, mCurrentRenderFrame};
    mCurrentScene->setCurrentRenderRange(mCurrRenderRange);
    mCurrentSoundComposition->startBlockingAtFrame(mCurrentRenderFrame);

    mMaxRenderFrame = mCurrentScene->getMaxFrame();
    setRenderingPreview(true);

    emit previewBeingRendered();
    if(TaskScheduler::sAllQuedCPUTasksFinished()) {
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
}

void RenderHandler::setPreviewing(const bool bT) {
    mPreviewing = bT;
    mCurrentScene->setPreviewing(bT);
}

void RenderHandler::interruptPreviewRendering() {
    setRenderingPreview(false);
    TaskScheduler::sClearAllFinishedFuncs();
    clearPreview();
    auto& cacheHandler = mCurrentScene->getCacheHandler();
    cacheHandler.blockConts({mSavedCurrentFrame + 1, mMaxRenderFrame}, false);
    setFrameAction(mSavedCurrentFrame);
    emit previewFinished();
}

void RenderHandler::interruptOutputRendering() {
    mCurrentScene->setOutputRendering(false);
    TaskScheduler::sClearAllFinishedFuncs();
    clearPreview();
    setFrameAction(mSavedCurrentFrame);
}

void RenderHandler::stopPreview() {
    setPreviewing(false);
    auto& cacheHandler = mCurrentScene->getCacheHandler();
    cacheHandler.blockConts({mSavedCurrentFrame + 1, mMaxRenderFrame}, false);
    setFrameAction(mSavedCurrentFrame);
    mCurrentScene->setCurrentPreviewContainer(mSavedCurrentFrame);
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
        if(TaskScheduler::sAllTasksFinished()) {
            playPreview();
        } else {
            const auto allFinishedFunc = [this]() {
                playPreview();
            };
            TaskScheduler::sSetAllTasksFinishedFunc(allFinishedFunc);
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
    mCurrentScene->setCurrentPreviewContainer(mCurrentPreviewFrame);
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
        mCurrentScene->setCurrentPreviewContainer(
                    mCurrentPreviewFrame);
        setFrameAction(mCurrentPreviewFrame);
    }
   emit mCurrentScene->requestUpdate();
}

void RenderHandler::nextSaveOutputFrame() {
    const auto& sCacheHandler = mCurrentScene->getSoundComposition()->getCacheHandler();
    const qreal fps = mCurrentScene->getFps();
    const int maxSec = qCeil(mMaxRenderFrame/fps);
    while(mCurrentEncodeSoundSecond <= maxSec) {
        const auto cont = sCacheHandler.atFrame(mCurrentEncodeSoundSecond);
        if(!cont) break;
        const auto sCont = GetAsSPtr(cont, SoundCacheContainer);
        const auto samples = sCont->getSamples();
        if(mCurrentEncodeSoundSecond == mFirstEncodeSoundSecond) {
            const int minSample = qRound(mMinRenderFrame*SOUND_SAMPLERATE/fps);
            const int max = samples->fSampleRange.fMax;
            VideoEncoder::sAddCacheContainerToEncoder(
                        SPtrCreate(Samples)(samples->mid({minSample, max})));
        } else {
            VideoEncoder::sAddCacheContainerToEncoder(
                        SPtrCreate(Samples)(samples));
        }
        sCont->setBlocked(false);
        mCurrentEncodeSoundSecond++;
    }

    const auto& cacheHandler = mCurrentScene->getCacheHandler();
    while(mCurrentEncodeFrame <= mMaxRenderFrame) {
        const auto cont = cacheHandler.atFrame(mCurrentEncodeFrame);
        if(!cont) break;
        VideoEncoder::sAddCacheContainerToEncoder(
                    GetAsSPtr(cont, ImageCacheContainer));
        mCurrentEncodeFrame = cont->getRangeMax() + 1;
    }

    //mCurrentScene->renderCurrentFrameToOutput(*mCurrentRenderSettings);
    if(mCurrentRenderFrame >= mMaxRenderFrame) {
        queTasksAndUpdate();
        if(TaskScheduler::sAllTasksFinished()) {
            TaskScheduler::sClearAllFinishedFuncs();
            mCurrentRenderSettings = nullptr;
            mCurrentScene->setOutputRendering(false);
            setFrameAction(mSavedCurrentFrame);
            if(qAbs(mSavedResolutionFraction -
                    mCurrentScene->getResolutionFraction()) > 0.1) {
                mCurrentScene->setResolutionFraction(mSavedResolutionFraction);
            }
            mCurrentSoundComposition->unblockAll();
            VideoEncoder::sFinishEncoding();
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
