#include "imagecachehandler.h"
#include "filecachehandler.h"

ImageCacheHandler::ImageCacheHandler() {}

ImageLoader::ImageLoader(const QString &filePath,
                         ImageCacheHandler * const handler) :
    mTargetHandler(handler), mFilePath(filePath) {

}

void ImageLoader::processTask() {
    const sk_sp<SkData> data = SkData::MakeFromFileName(
                mFilePath.toLocal8Bit().data());
    mImage = SkImage::MakeFromEncoded(data);
}

void ImageLoader::afterProcessing() {
    mTargetHandler->setImage(mImage);
}
