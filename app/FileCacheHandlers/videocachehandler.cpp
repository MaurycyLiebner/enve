#include "videocachehandler.h"
#include "Boxes/boundingboxrendercontainer.h"
#include "Boxes/videobox.h"

#include <QFileDialog>
#include "GUI/mainwindow.h"
#include "filesourcescache.h"

VideoCacheHandler::VideoCacheHandler(const QString &filePath) :
    AnimationCacheHandler(filePath) {
    openVideoStream();
}

sk_sp<SkImage> VideoCacheHandler::getFrameAtFrame(const int &relFrame) {
    const auto cont = mFramesCache.getRenderContainerAtRelFrame
            <ImageCacheContainer>(relFrame);
    if(!cont) return sk_sp<SkImage>();
    return cont->getImageSk();
}

sk_sp<SkImage> VideoCacheHandler::getFrameAtOrBeforeFrame(const int& relFrame) {
    const auto cont = mFramesCache.getRenderContainerAtOrBeforeRelFrame
            <ImageCacheContainer>(relFrame);
    if(!cont) return sk_sp<SkImage>();
    return cont->getImageSk();
}

void VideoCacheHandler::frameLoaderFinished(const int& frame,
                                            const sk_sp<SkImage>& image) {
    if(image) {
        mFramesCache.createNewRenderContainerAtRelFrame
                <ImageCacheContainer>(frame, image);
    } else {
        mFrameCount = frame;
        for(const auto &box : mDependentBoxes) {
            if(!box) continue;
            const auto vidBox = GetAsPtr(box, VideoBox);
            vidBox->updateDurationRectangleAnimationRange();
        }
    }
    removeFrameLoader(frame);
}

void VideoCacheHandler::clearCache() {
    mFramesCache.clearCache();
    FileCacheHandler::clearCache();
}

void VideoCacheHandler::replace() {
    const QString importPath = QFileDialog::getOpenFileName(
                MainWindow::getInstance(),
                "Replace Video Source " + mFilePath, "",
                "Files (*.mp4 *.mov *.avi *.mkv *.m4v)");
    MainWindow::getInstance()->enableEventFilter();
    if(!importPath.isEmpty()) {
        const QFile file(importPath);
        if(!file.exists()) return;
        if(hasVideoExt(importPath)) {
            mFilePath = importPath;
            openVideoStream();
            clearCache();
        }
    }
}

_ScheduledTask* VideoCacheHandler::scheduleFrameLoad(const int &frame) {
    if(frame < 0 || frame >= mFrameCount)
        RuntimeThrow("Frame outside of range " + std::to_string(frame));
    const auto currLoader = getFrameLoader(frame);
    if(currLoader) return currLoader;
    const auto contAtFrame = mFramesCache.getRenderContainerAtRelFrame
            <ImageCacheContainer>(frame);
    if(contAtFrame) return contAtFrame->scheduleLoadFromTmpFile();
    const auto loader = addFrameLoader(frame);
    loader->scheduleTask();
    return loader;
}

void VideoFrameLoader::afterProcessingFinished() {
    mCacheHandler->frameLoaderFinished(mFrameId, mLoadedFrame);
    _HDDTask::afterProcessingFinished();
}

void VideoFrameLoader::afterCanceled() {
    mCacheHandler->frameLoaderCanceled(mFrameId);
    _HDDTask::afterCanceled();
}
