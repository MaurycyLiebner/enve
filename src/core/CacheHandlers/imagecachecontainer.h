#ifndef IMAGECACHECONTAINER_H
#define IMAGECACHECONTAINER_H
#include "skia/skiaincludes.h"
#include "hddcachablerangecont.h"
class Canvas;

class ImageCacheContainer : public HddCachableRangeCont {
    e_OBJECT
protected:
    ImageCacheContainer(const FrameRange& range,
                        HddCachableCacheHandler * const parent);
    ImageCacheContainer(const sk_sp<SkImage>& img,
                        const FrameRange &range,
                        HddCachableCacheHandler * const parent);
    stdsptr<eHddTask> createTmpFileDataSaver();
    stdsptr<eHddTask> createTmpFileDataLoader();
    int clearMemory();
public:
    int getByteCount();

    void drawSk(SkCanvas * const canvas, const SkFilterQuality filter);

    sk_sp<SkImage> getImageSk();

    void setDataLoadedFromTmpFile(const sk_sp<SkImage> &img);
    void replaceImageSk(const sk_sp<SkImage> &img);
protected:
    sk_sp<SkImage> mImageSk;
};


#endif // IMAGECACHECONTAINER_H
