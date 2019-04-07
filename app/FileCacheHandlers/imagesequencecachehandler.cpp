#include "imagesequencecachehandler.h"
#include "filesourcescache.h"

ImageSequenceCacheHandler::ImageSequenceCacheHandler(
        const QStringList &framePaths) {
    mFramePaths = framePaths;
    for(const QString &path : framePaths) {
        auto imgCacheHandler = GetAsPtr(
                FileSourcesCache::getHandlerForFilePath(path),
                    ImageCacheHandler);
        if(imgCacheHandler) {
            mFrameImageHandlers << GetAsPtr(imgCacheHandler, ImageCacheHandler);
        } else {
            auto newHandler = FileSourcesCache::
                    createNewHandler<ImageCacheHandler>(path, false);
            mFrameImageHandlers << newHandler;
        }
    }
    updateFrameCount();
}

sk_sp<SkImage> ImageSequenceCacheHandler::getFrameAtFrame(const int &relFrame) {
    ImageCacheHandler *cacheHandler = mFrameImageHandlers.at(relFrame);
    if(!cacheHandler) return sk_sp<SkImage>();
    return cacheHandler->getImage();
}

sk_sp<SkImage> ImageSequenceCacheHandler::getFrameAtOrBeforeFrame(
        const int &relFrame) {
    if(mFrameImageHandlers.isEmpty()) return sk_sp<SkImage>();
    if(relFrame >= mFrameImageHandlers.count()) {
        return mFrameImageHandlers.last()->getImage();
    }
    ImageCacheHandler *cacheHandler = mFrameImageHandlers.at(relFrame);
    return cacheHandler->getImage();
}

void ImageSequenceCacheHandler::updateFrameCount() {
    mFrameCount = mFramePaths.count();
}

void ImageSequenceCacheHandler::clearCache() {
    for(const auto &cacheHandler : mFrameImageHandlers) {
        cacheHandler->clearCache();
    }
    FileCacheHandler::clearCache();
}

_ScheduledTask *ImageSequenceCacheHandler::scheduleFrameLoad(
        const int &frame) {
    const auto& imageHandler = mFrameImageHandlers.at(frame);
    if(imageHandler->hasImage()) return nullptr;
    return imageHandler->scheduleLoad();
}
