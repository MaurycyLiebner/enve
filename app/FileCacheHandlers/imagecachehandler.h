#ifndef IMAGECACHEHANDLER_H
#define IMAGECACHEHANDLER_H
#include "skia/skiahelpers.h"
#include "filecachehandler.h"
class ImageCacheHandler;
class ImageLoader : public HDDTask {
    friend class StdSelfRef;
protected:
    ImageLoader(const QString &filePath,
                ImageCacheHandler * const handler);
public:
    void processTask();
    void afterProcessing();
    void afterCanceled();
private:
    ImageCacheHandler * const mTargetHandler;
    const QString mFilePath;
    sk_sp<SkImage> mImage;
};

class ImageCacheHandler : public FileDataCacheHandler {
    friend class SelfRef;
    friend class ImageLoader;
protected:
    ImageCacheHandler();
public:
    void replace() {}
    void afterSourceChanged() {}

    void clearCache() {
        mImage.reset();
        mImageLoader.reset();
    }

    ImageLoader * scheduleLoad() {
        if(mImage) return nullptr;
        if(mImageLoader) return mImageLoader.get();
        mImageLoader = SPtrCreate(ImageLoader)(mFilePath, this);
        mImageLoader->scheduleTask();
        return mImageLoader.get();
    }

    bool hasImage() const {
        return mImage.get();
    }

    sk_sp<SkImage> getImage() { return mImage; }
    sk_sp<SkImage> getImageCopy() {
        return SkiaHelpers::makeCopy(mImage);
    }
protected:
    void setImage(const sk_sp<SkImage>& img) {
        mImage = img;
        mImageLoader.reset();
    }
private:
    sk_sp<SkImage> mImage;
    stdsptr<ImageLoader> mImageLoader;
};

#endif // IMAGECACHEHANDLER_H
