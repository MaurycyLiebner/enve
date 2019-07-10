#include "imagecachehandler.h"
#include "filecachehandler.h"

ImageDataHandler::ImageDataHandler() {}

ImageLoader *ImageDataHandler::scheduleLoad() {
    if(mImage) return nullptr;
    if(mImageLoader) return mImageLoader.get();
    mImageLoader = SPtrCreate(ImageLoader)(mFilePath, this);
    mImageLoader->scheduleTask();
    return mImageLoader.get();
}

ImageLoader::ImageLoader(const QString &filePath,
                         ImageDataHandler * const handler) :
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

void ImageLoader::afterCanceled() {
    mTargetHandler->setImage(mImage);
}
