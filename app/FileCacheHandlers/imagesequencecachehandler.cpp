#include "imagesequencecachehandler.h"
#include "filesourcescache.h"

ImageSequenceCacheHandler::ImageSequenceCacheHandler() {}

void ImageSequenceCacheHandler::setFolderPath(const QString &folderPath) {
    mFileHandler = FileCacheHandler::sGetFileHandler<ImageSequenceFileHandler>(folderPath);
}

void ImageSequenceFileHandler::afterPathSet(const QString &folderPath) {
    Q_UNUSED(folderPath);
    reload();
}

sk_sp<SkImage> ImageSequenceFileHandler::getFrameAtFrame(const int relFrame) {
    if(mFrameImageHandlers.isEmpty()) return nullptr;
    const auto cacheHandler = mFrameImageHandlers.at(relFrame);
    if(!cacheHandler) return sk_sp<SkImage>();
    return cacheHandler->getImage();
}

sk_sp<SkImage> ImageSequenceFileHandler::getFrameAtOrBeforeFrame(
        const int relFrame) {
    if(mFrameImageHandlers.isEmpty()) return nullptr;
    if(relFrame >= mFrameImageHandlers.count()) {
        return mFrameImageHandlers.last()->getImage();
    }
    const auto cacheHandler = mFrameImageHandlers.at(relFrame);
    return cacheHandler->getImage();
}

Task *ImageSequenceFileHandler::scheduleFrameLoad(const int frame) {
    if(mFrameImageHandlers.isEmpty()) return nullptr;
    const auto& imageHandler = mFrameImageHandlers.at(frame);
    if(imageHandler->hasImage()) return nullptr;
    return imageHandler->scheduleLoad();
}

void ImageSequenceFileHandler::reload() {
    mFrameImageHandlers.clear();
    QDir dir(mFolderPath);
    mFileMissing = !dir.exists();
    if(mFileMissing) return;
    dir.setFilter(QDir::Files);
    dir.setSorting(QDir::Name);
    const auto files = dir.entryInfoList();
    for(const auto& fileInfo : files) {
        if(!isImageExt(fileInfo.suffix())) continue;
        const auto filePath = fileInfo.absoluteFilePath();
        const auto handler = ImageDataHandler::sGetCreateDataHandler<ImageDataHandler>(filePath);
        handler->clearCache();
        mFrameImageHandlers << handler;
    }
    if(mFrameImageHandlers.isEmpty()) mFileMissing = true;
}

#include <QFileDialog>
#include "GUI/mainwindow.h"
void ImageSequenceFileHandler::replace(QWidget* const parent) {
    const auto dir = QFileDialog::getExistingDirectory(
                parent, "Import Image Sequence", mPath);
    if(!dir.isEmpty()) setPath(dir);
}
