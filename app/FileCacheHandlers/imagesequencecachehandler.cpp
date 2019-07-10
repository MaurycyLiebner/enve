#include "imagesequencecachehandler.h"
#include "filesourcescache.h"

ImageSequenceCacheHandler::ImageSequenceCacheHandler() {}

void ImageSequenceCacheHandler::setFolderPath(const QString &folderPath) {
    mFileHandler = FileCacheHandler::sGetFileHandler<ImageSequenceFileHandler>(folderPath);
}

void ImageSequenceFileHandler::afterPathSet(const QString &folderPath) {
    mFolderPath = folderPath;
    reload();
}

sk_sp<SkImage> ImageSequenceFileHandler::getFrameAtFrame(const int relFrame) {
    const auto cacheHandler = mFrameImageHandlers.at(relFrame);
    if(!cacheHandler) return sk_sp<SkImage>();
    return cacheHandler->getImage();
}

sk_sp<SkImage> ImageSequenceFileHandler::getFrameAtOrBeforeFrame(
        const int relFrame) {
    if(mFrameImageHandlers.isEmpty()) return sk_sp<SkImage>();
    if(relFrame >= mFrameImageHandlers.count()) {
        return mFrameImageHandlers.last()->getImage();
    }
    const auto cacheHandler = mFrameImageHandlers.at(relFrame);
    return cacheHandler->getImage();
}

Task *ImageSequenceFileHandler::scheduleFrameLoad(const int frame) {
    const auto& imageHandler = mFrameImageHandlers.at(frame);
    if(imageHandler->hasImage()) return nullptr;
    return imageHandler->scheduleLoad();
}

void ImageSequenceFileHandler::reload() {
    mFrameImageHandlers.clear();
    QDir dir(mFolderPath);
    if(!dir.exists()) return;
    dir.setFilter(QDir::Files);
    dir.setSorting(QDir::Name);
    const auto files = dir.entryInfoList();
    for(const auto& fileInfo : files) {
        const auto filePath = fileInfo.absoluteFilePath();
        const auto handler = ImageDataHandler::sGetCreateDataHandler<ImageDataHandler>(filePath);
        handler->clearCache();
        mFrameImageHandlers << handler;
    }
}

void ImageSequenceFileHandler::replace() {

}
