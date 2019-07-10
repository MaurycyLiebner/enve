#include "videocachehandler.h"
#include "Boxes/boundingboxrendercontainer.h"
#include "Boxes/videobox.h"

#include <QFileDialog>
#include "GUI/mainwindow.h"
#include "filesourcescache.h"

#include "videoframeloader.h"

VideoFrameHandler::VideoFrameHandler(VideoDataHandler * const cacheHandler) :
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

VideoDataHandler *VideoFrameHandler::getDataHandler() const {
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

VideoFrameLoader *VideoFrameHandler::addFrameLoader(const int frameId,
                                                    AVFrame * const frame) {
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
    const auto currLoader = getFrameLoader(frame);
    if(currLoader) return currLoader;
    if(mDataHandler->getFrameAtFrame(frame)) return nullptr;
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
    openVideoStream();
}

void VideoFrameHandler::afterSourceChanged() {
    openVideoStream();
}

void VideoDataHandler::clearCache() {
    mFramesCache.clear();
    mFramesBeingLoaded.clear();
    const auto frameLoaders = mFrameLoaders;
    for(const auto& loader : frameLoaders)
        loader->cancel();
    mFrameLoaders.clear();
}

void VideoDataHandler::replace() {
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

void VideoDataHandler::afterSourceChanged() {
    for(const auto& handler : mFrameHandlers) {
        handler->afterSourceChanged();
    }
}

const HDDCachableCacheHandler &VideoDataHandler::getCacheHandler() const {
    return mFramesCache;
}

void VideoDataHandler::addFrameLoader(const int frameId,
                                            const stdsptr<VideoFrameLoader> &loader) {
    mFramesBeingLoaded << frameId;
    mFrameLoaders << loader;
}

VideoFrameLoader *VideoDataHandler::getFrameLoader(const int frame) const {
    const int id = mFramesBeingLoaded.indexOf(frame);
    if(id >= 0) return mFrameLoaders.at(id).get();
    return nullptr;
}

void VideoDataHandler::removeFrameLoader(const int frame) {
    const int id = mFramesBeingLoaded.indexOf(frame);
    if(id < 0 || id >= mFramesBeingLoaded.count()) return;
    mFramesBeingLoaded.removeAt(id);
    mFrameLoaders.removeAt(id);
}

void VideoDataHandler::frameLoaderFinished(const int frame,
                                                 const sk_sp<SkImage> &image) {
    if(image) {
        mFramesCache.add(SPtrCreate(ImageCacheContainer)(
                             image, FrameRange{frame, frame}, &mFramesCache));
    } else {
        mFrameCount = frame;
        emit frameCountUpdated(mFrameCount);
    }
}

Task *VideoDataHandler::scheduleFrameHddCacheLoad(const int frame) {
    const auto contAtFrame = mFramesCache.atFrame<ImageCacheContainer>(frame);
    if(contAtFrame) return contAtFrame->scheduleLoadFromTmpFile();
    return nullptr;
}

sk_sp<SkImage> VideoDataHandler::getFrameAtFrame(const int relFrame) const {
    const auto cont = mFramesCache.atFrame<ImageCacheContainer>(relFrame);
    if(!cont) return sk_sp<SkImage>();
    return cont->getImageSk();
}

sk_sp<SkImage> VideoDataHandler::getFrameAtOrBeforeFrame(const int relFrame) const {
    const auto cont = mFramesCache.atOrBeforeFrame<ImageCacheContainer>(relFrame);
    if(!cont) return sk_sp<SkImage>();
    return cont->getImageSk();
}

int VideoDataHandler::getFrameCount() const { return mFrameCount; }

void VideoDataHandler::setFrameCount(const int count) { mFrameCount = count; }


bool hasSound(const char* path) {
    // get format from audio file
    AVFormatContext* format = avformat_alloc_context();
    if(avformat_open_input(&format, path, nullptr, nullptr) != 0) {
        RuntimeThrow("Could not open file " + path);
    }
    if(avformat_find_stream_info(format, nullptr) < 0) {
        RuntimeThrow("Could not retrieve stream info from file " + path);
    }

    // Find the index of the first audio stream
    for(uint i = 0; i < format->nb_streams; i++) {
        AVStream *streamT = format->streams[i];
        const AVMediaType &mediaType = streamT->codecpar->codec_type;
        if(mediaType == AVMEDIA_TYPE_AUDIO) {
            return true;
        }
    }

    avformat_free_context(format);

    // success
    return false;
}

void VideoFileHandler::afterPathSet(const QString &path) {
    mDataHandler = VideoDataHandler::sGetCreateDataHandler<VideoDataHandler>(path);
    if(!hasSound(path.toLatin1().data())) return mSoundHandler.reset();
    mSoundHandler = SoundDataHandler::sGetCreateDataHandler<SoundDataHandler>(path);
}
