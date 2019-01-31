#include "imagecachehandler.h"
#include "filecachehandler.h"

ImageCacheHandler::ImageCacheHandler(const QString &filePath,
                                     const bool &visibleSeparatly) :
    FileCacheHandler(filePath) {
    mVisibleInListWidgets = visibleSeparatly;
}

void ImageCacheHandler::_processUpdate() {
    sk_sp<SkData> data = SkData::MakeFromFileName(
                mFilePath.toLocal8Bit().data());
    mUpdateImage = SkImage::MakeFromEncoded(data);
}

void ImageCacheHandler::afterProcessingFinished() {
    mImage = mUpdateImage;
    mUpdateImage.reset();
}
