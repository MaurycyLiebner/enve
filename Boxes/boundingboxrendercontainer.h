#ifndef BOUNDINGBOXRENDERCONTAINER_H
#define BOUNDINGBOXRENDERCONTAINER_H
class BoundingBox;
#include "selfref.h"
#include "skqtconversions.h"
#include "skiaincludes.h"

class CacheHandler;
struct BoundingBoxRenderData;

class MinimalCacheContainer : public StdSelfRef {
public:
    MinimalCacheContainer();

    virtual ~MinimalCacheContainer();

    virtual bool freeThis() = 0;

    void thisAccessed();

    virtual int getByteCount() = 0;

    void setBlocked(const bool &bT) {
        mBlocked = bT;
    }
protected:
    bool mBlocked = false;
};

class CacheContainer : public MinimalCacheContainer {
public:
    CacheContainer() {}

    void setParentCacheHandler(CacheHandler *handler);
    bool freeThis();

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
    void drawCacheOnTimeline(QPainter *p,
                             const qreal &pixelsPerFrame,
                             const qreal &drawY,
                             const int &startFrame,
                             const int &endFrame);

    virtual void drawSk(SkCanvas *canvas);
protected:
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
