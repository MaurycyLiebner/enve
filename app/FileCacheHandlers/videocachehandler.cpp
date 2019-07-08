#include "videocachehandler.h"
#include "Boxes/boundingboxrendercontainer.h"
#include "Boxes/videobox.h"

#include <QFileDialog>
#include "GUI/mainwindow.h"
#include "filesourcescache.h"

#include "videoframeloader.h"

VideoFrameHandler::VideoFrameHandler(VideoFrameCacheHandler * const cacheHandler) :
    mDataHandler(cacheHandler) {
    openVideoStream();
}

sk_sp<SkImage> VideoFrameHandler::getFrameAtFrame(const int relFrame) {
    return mDataHandler->getFrameAtFrame(relFrame);
}

sk_sp<SkImage> VideoFrameHandler::getFrameAtOrBeforeFrame(const int relFrame) {
    return mDataHandler->getFrameAtOrBeforeFrame(relFrame);
}

void VideoFrameHandler::frameLoaderFinished(const int frame,
                                            const sk_sp<SkImage>& image) {
    mDataHandler->frameLoaderFinished(frame, image);
    removeFrameLoader(frame);
}

void VideoFrameHandler::frameLoaderCanceled(const int frameId) {
    removeFrameLoader(frameId);
}

VideoFrameCacheHandler *VideoFrameHandler::getDataHandler() const {
    return mDataHandler;
}

const HDDCachableCacheHandler &VideoFrameHandler::getCacheHandler() const {
    return mDataHandler->getCacheHandler();
}

VideoFrameLoader *VideoFrameHandler::getFrameLoader(const int frame) {
    return mDataHandler->getFrameLoader(frame);
}

VideoFrameLoader *VideoFrameHandler::addFrameLoader(const int frameId) {
    const auto loader = SPtrCreate(VideoFrameLoader)(
                    this, mVideoStreamsData, frameId);
        mDataHandler->addFrameLoader(frameId, loader);
        for(const auto& nFrame : mNeededFrames) {
            const auto nLoader = getFrameLoader(nFrame);
            if(nFrame < frameId) nLoader->addDependent(loader.get());
            else loader->addDependent(nLoader);
        }
        mNeededFrames.insert(frameId);

        return loader.get();
}

VideoFrameLoader *VideoFrameHandler::addFrameLoader(const int frameId, AVFrame * const frame) {
    const auto loader = SPtrCreate(VideoFrameLoader)(
                    this, mVideoStreamsData, frameId, frame);
        mDataHandler->addFrameLoader(frameId, loader);
        return loader.get();
}

void VideoFrameHandler::removeFrameLoader(const int frame) {
    mDataHandler->removeFrameLoader(frame);
        mNeededFrames.erase(frame);
}

void VideoFrameHandler::openVideoStream() {
    const auto filePath = mDataHandler->getFilePath();
    mVideoStreamsData = VideoStreamsData::sOpen(filePath);
    mDataHandler->setFrameCount(mVideoStreamsData->fFrameCount);
}

Task* VideoFrameHandler::scheduleFrameLoad(const int frame) {
    if(frame < 0 || frame >= getFrameCount())
        RuntimeThrow("Frame outside of range " + std::to_string(frame));
    if(mDataHandler->getFrameAtFrame(frame)) return nullptr;
    const auto currLoader = getFrameLoader(frame);
    if(currLoader) return currLoader;
    const auto loadTask = mDataHandler->scheduleFrameHddCacheLoad(frame);
    if(loadTask) return loadTask;
    const auto loader = addFrameLoader(frame);
    loader->scheduleTask();
    return loader;
}

int VideoFrameHandler::getFrameCount() const {
    return mDataHandler->getFrameCount();
}

void VideoFrameHandler::reload() {
    mDataHandler->clearCache();
}

void VideoFrameHandler::afterPathChanged() {
    openVideoStream();
}

void VideoFrameCacheHandler::clearCache() {
    mFramesCache.clear();
}

void VideoFrameCacheHandler::replace() {
    const QString importPath = QFileDialog::getOpenFileName(
                MainWindow::getInstance(),
                "Replace Video Source " + mFilePath, "",
                "Files (*.mp4 *.mov *.avi *.mkv *.m4v)");
    MainWindow::getInstance()->enableEventFilter();
    if(!importPath.isEmpty()) {
        const QFile file(importPath);
        if(!file.exists()) return;
        if(hasVideoExt(importPath)) {
            try {
                setFilePath(importPath);
            } catch(const std::exception& e) {
                gPrintExceptionCritical(e);
            }
        }
    }
}

void VideoFrameCacheHandler::afterPathChanged() {
    for(const auto& handler : mFrameHandlers) {
        handler->afterPathChanged();
    }
}

const HDDCachableCacheHandler &VideoFrameCacheHandler::getCacheHandler() const {
    return mFramesCache;
}

void VideoFrameCacheHandler::addFrameLoader(const int frameId, const stdsptr<VideoFrameLoader> &loader) {
    mFramesBeingLoaded << frameId;
    mFrameLoaders << loader;
}

VideoFrameLoader *VideoFrameCacheHandler::getFrameLoader(const int frame) const {
    const int id = mFramesBeingLoaded.indexOf(frame);
    if(id >= 0) return mFrameLoaders.at(id).get();
    return nullptr;
}

void VideoFrameCacheHandler::removeFrameLoader(const int frame) {
    const int id = mFramesBeingLoaded.indexOf(frame);
    if(id < 0 || id >= mFramesBeingLoaded.count()) return;
    mFramesBeingLoaded.removeAt(id);
    mFrameLoaders.removeAt(id);
}

void VideoFrameCacheHandler::frameLoaderFinished(const int frame, const sk_sp<SkImage> &image) {
    if(image) {
        mFramesCache.add(SPtrCreate(ImageCacheContainer)(
                             image, FrameRange{frame, frame}, &mFramesCache));
    } else {
        mFrameCount = frame;
        for(const auto &box : mDependentBoxes) {
            if(!box) continue;
            const auto vidBox = GetAsPtr(box, VideoBox);
            vidBox->updateDurationRectangleAnimationRange();
        }
    }
}

Task *VideoFrameCacheHandler::scheduleFrameHddCacheLoad(const int frame) {
    const auto contAtFrame = mFramesCache.atFrame<ImageCacheContainer>(frame);
    if(contAtFrame) return contAtFrame->scheduleLoadFromTmpFile();
    return nullptr;
}

sk_sp<SkImage> VideoFrameCacheHandler::getFrameAtFrame(const int relFrame) const {
    const auto cont = mFramesCache.atFrame<ImageCacheContainer>(relFrame);
    if(!cont) return sk_sp<SkImage>();
    return cont->getImageSk();
}

sk_sp<SkImage> VideoFrameCacheHandler::getFrameAtOrBeforeFrame(const int relFrame) const {
    const auto cont = mFramesCache.atOrBeforeFrame<ImageCacheContainer>(relFrame);
    if(!cont) return sk_sp<SkImage>();
    return cont->getImageSk();
}

int VideoFrameCacheHandler::getFrameCount() const { return mFrameCount; }

void VideoFrameCacheHandler::setFrameCount(const int count) { mFrameCount = count; }
