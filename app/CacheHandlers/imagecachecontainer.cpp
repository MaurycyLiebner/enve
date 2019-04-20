#include "imagecachecontainer.h"
#include "tmpfilehandlers.h"
#include "canvas.h"
#include "skia/skiahelpers.h"

ImageCacheContainer::ImageCacheContainer(const FrameRange &range,
                                         RangeCacheHandler * const parent) :
    Base(range, parent) {}

ImageCacheContainer::ImageCacheContainer(const sk_sp<SkImage> &img,
                                         const FrameRange &range,
                                         RangeCacheHandler * const parent) :
    ImageCacheContainer(range, parent) {
    replaceImageSk(img);
}

void ImageCacheContainer::replaceImageSk(const sk_sp<SkImage> &img) {
    mImageSk = img;
    afterDataReplaced();
}

int ImageCacheContainer::getByteCount() {
    if(!mImageSk) return 0;
    SkPixmap pixmap;
    if(mImageSk->peekPixels(&pixmap)) {
        return pixmap.width()*pixmap.height()*
                pixmap.info().bytesPerPixel();
    }
    return 0;
}

sk_sp<SkImage> ImageCacheContainer::getImageSk() {
    return mImageSk;
}

void ImageCacheContainer::setDataLoadedFromTmpFile(const sk_sp<SkImage> &img) {
    replaceImageSk(img);

    if(mTmpLoadTargetCanvas) {
        mTmpLoadTargetCanvas->setCurrentPreviewContainer(
                    ref<ImageCacheContainer>());
        mTmpLoadTargetCanvas = nullptr;
    }
    afterDataLoadedFromTmpFile();
}

void ImageCacheContainer::drawSk(SkCanvas * const canvas, SkPaint *paint,
                                 GrContext * const grContext) {
    Q_UNUSED(paint);
    Q_UNUSED(grContext);
    //SkPaint paint;
    //paint.setAntiAlias(true);
    //paint.setFilterQuality(kHigh_SkFilterQuality);
    SkiaHelpers::drawImageGPU(canvas, mImageSk, 0, 0, paint, grContext);
    //canvas->drawImage(mImageSk, 0, 0/*, &paint*/);
}

int ImageCacheContainer::clearMemory() {
    const int bytes = getByteCount();
    mImageSk.reset();
    setDataInMemory(false);
    return bytes;
}

void ImageCacheContainer::setLoadTargetCanvas(Canvas *canvas) {
    mTmpLoadTargetCanvas = canvas;
}

stdsptr<_HDDTask> ImageCacheContainer::createTmpFileDataSaver() {
    return SPtrCreate(CacheContainerTmpFileDataSaver)(mImageSk, this);
}

stdsptr<_HDDTask> ImageCacheContainer::createTmpFileDataLoader() {
    return SPtrCreate(CacheContainerTmpFileDataLoader)(mTmpFile, this);
}
