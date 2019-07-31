#ifndef IMAGECACHECONTAINER_H
#define IMAGECACHECONTAINER_H
#include "skia/skiaincludes.h"
#include "hddcachablerangecont.h"
class Canvas;

class ImageCacheContainer : public HDDCachableRangeCont {
    friend class StdSelfRef;
protected:
    ImageCacheContainer(const FrameRange& range,
                        HDDCachableCacheHandler * const parent);
    ImageCacheContainer(const sk_sp<SkImage>& img,
                        const FrameRange &range,
                        HDDCachableCacheHandler * const parent);
    stdsptr<HDDTask> createTmpFileDataSaver();
    stdsptr<HDDTask> createTmpFileDataLoader();
    int clearMemory();
public:
    int getByteCount();

    void drawSk(SkCanvas * const canvas, GrContext* const grContext);
    void drawSk(SkCanvas * const canvas, GrContext* const grContext,
                SkPaint &paint);

    sk_sp<SkImage> getImageSk();

    void setDataLoadedFromTmpFile(const sk_sp<SkImage> &img);
    void replaceImageSk(const sk_sp<SkImage> &img);

    void setLoadTargetCanvas(Canvas *canvas);
protected:
    qptr<Canvas> mTmpLoadTargetCanvas;
    sk_sp<SkImage> mImageSk;
};


#endif // IMAGECACHECONTAINER_H
