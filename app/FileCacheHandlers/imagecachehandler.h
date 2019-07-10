#ifndef IMAGECACHEHANDLER_H
#define IMAGECACHEHANDLER_H
#include "skia/skiahelpers.h"
#include "filecachehandler.h"
#include "updatable.h"
class ImageDataHandler;
class ImageLoader : public HDDTask {
    friend class StdSelfRef;
protected:
    ImageLoader(const QString &filePath,
                ImageDataHandler * const handler);
public:
    void processTask();
    void afterProcessing();
    void afterCanceled();
private:
    ImageDataHandler * const mTargetHandler;
    const QString mFilePath;
    sk_sp<SkImage> mImage;
};

class ImageDataHandler : public FileDataCacheHandler {
    friend class SelfRef;
    friend class ImageLoader;
protected:
    ImageDataHandler();
public:
    void afterSourceChanged() {}

    void clearCache() {
        mImage.reset();
        mImageLoader.reset();
    }

    ImageLoader * scheduleLoad();
    bool hasImage() const { return mImage.get(); }
    sk_sp<SkImage> getImage() const { return mImage; }
    sk_sp<SkImage> getImageCopy() const {
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

class ImageFileHandler : public FileCacheHandler {
    friend class SelfRef;
protected:
    ImageFileHandler() {}

    void afterPathSet(const QString& path) {
        const auto current = ImageDataHandler::sGetDataHandler<ImageDataHandler>(path);
        if(current) mDataHandler = GetAsSPtr(current, ImageDataHandler);
        else mDataHandler = ImageDataHandler::sCreateDataHandler<ImageDataHandler>(path);
    }
public:
    void reload() {}
    void replace() {}

    ImageLoader * scheduleLoad() {
        if(!mDataHandler) return nullptr;
        return mDataHandler->scheduleLoad();
    }

    bool hasImage() const {
        if(!mDataHandler) return false;
        return mDataHandler->hasImage();
    }

    sk_sp<SkImage> getImage() const {
        if(!mDataHandler) return nullptr;
        return mDataHandler->getImage();
    }
    sk_sp<SkImage> getImageCopy() const {
        if(!mDataHandler) return nullptr;
        return mDataHandler->getImageCopy();
    }
private:
    qsptr<ImageDataHandler> mDataHandler;
};

#endif // IMAGECACHEHANDLER_H
