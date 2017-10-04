#ifndef BOUNDINGBOXRENDERCONTAINER_H
#define BOUNDINGBOXRENDERCONTAINER_H
class BoundingBox;
#include "selfref.h"
#include "skqtconversions.h"
#include "skiaincludes.h"
class QTemporaryFile;

class CacheHandler;
struct BoundingBoxRenderData;

class MinimalCacheContainer : public StdSelfRef {
public:
    MinimalCacheContainer(const bool &addToMemoryHandler = true);

    virtual ~MinimalCacheContainer();

    bool freeAndRemoveFromMemoryHandler();

    virtual bool freeThis() = 0;

    virtual int getByteCount() = 0;

    virtual void setBlocked(const bool &bT) {
        mBlocked = bT;
    }

    void setHandledByMemoryHanlder(const bool &bT) {
        mHandledByMemoryHandler = bT;
    }

    const bool &handledByMemoryHandler() {
        return mHandledByMemoryHandler;
    }
protected:
    bool mHandledByMemoryHandler = false;
    bool mBlocked = false;
};

class CacheContainer : public MinimalCacheContainer {
public:
    CacheContainer() {}
    ~CacheContainer();

    void setParentCacheHandler(CacheHandler *handler);
    bool freeThis();
    void setBlocked(const bool &bT);

    void neededInMemory() {
        if(mNoDataInMemory) {
            loadFromTmpFile();
        }
    }

    int getByteCount() {
        if(mImageSk.get() == NULL) return 0;
        SkPixmap pixmap;
        if(mImageSk->peekPixels(&pixmap)) {
            return pixmap.width()*pixmap.height()*
                    pixmap.info().bytesPerPixel();
        }
        return 0;
    }

    sk_sp<SkImage> getImageSk() {
        return mImageSk;
    }

    void replaceImageSk(const sk_sp<SkImage> &img);

    const int &getMinRelFrame() const;

    const int &getMaxRelFrame() const;

    void setRelFrame(const int &frame);
    void setMaxRelFrame(const int &maxFrame);
    void setMinRelFrame(const int &minFrame);
    void setRelFrameRange(const int &minFrame, const int &maxFrame);
    bool relFrameInRange(const int &relFrame);
    virtual void drawSk(SkCanvas *canvas);
    bool storesDataInMemory() {
        return !mNoDataInMemory;
    }
protected:
    void loadFromTmpFile();
    void saveToTmpFile();
    bool mNoDataInMemory = false;
    QTemporaryFile *mTmpFile = NULL;

    sk_sp<SkImage> mImageSk;
    CacheHandler *mParentCacheHandler = NULL;
    int mMinRelFrame = 0;
    int mMaxRelFrame = 0;
};

class RenderContainer : public CacheContainer {
public:
    RenderContainer() {}
    virtual ~RenderContainer();

    void drawSk(SkCanvas *canvas, SkPaint *paint);

    void updatePaintTransformGivenNewCombinedTransform(
            const QMatrix &combinedTransform);

    void setTransform(const QMatrix &transform);

    const QMatrix &getTransform() const;

    const QMatrix &getPaintTransform() const;

    const SkPoint &getDrawpos() const;

    const qreal &getResolutionFraction() const;

    void setVariablesFromRenderData(BoundingBoxRenderData *data);
    int getRelFrame() {
        return mRelFrame;
    }

    BoundingBoxRenderData *getSrcRenderData() {
        return mSrcRenderData.get();
    }

protected:
    std::shared_ptr<BoundingBoxRenderData> mSrcRenderData;
    int mRelFrame = 0;
    qreal mResolutionFraction;
    QMatrix mTransform;
    QMatrix mPaintTransform;
    SkPoint mDrawPos;
};

#endif // BOUNDINGBOXRENDERCONTAINER_H
