#ifndef BOUNDINGBOXRENDERCONTAINER_H
#define BOUNDINGBOXRENDERCONTAINER_H
class BoundingBox;
#include <QImage>
#include "selfref.h"
#include "skqtconversions.h"
#include "skiaincludes.h"

class CacheHandler;
struct BoundingBoxRenderData;

class CacheContainer : public SimpleSmartPointer {
public:
    CacheContainer();

    virtual ~CacheContainer();

    void setParentCacheHandler(CacheHandler *handler);
    bool freeThis();

    void thisAccessed();

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

    void setBlocked(const bool &bT) {
        if(bT == mBlocked) return;
        mBlocked = bT;
        if(bT) {
            incNumberPointers();
        } else {
            decNumberPointers();
        }
    }

protected:
    bool mBlocked = false;
    sk_sp<SkImage> mImageSk;
    CacheHandler *mParentCacheHandler = NULL;
    int mMinRelFrame = 0;
    int mMaxRelFrame = 0;
};

class RenderContainer : public CacheContainer {
public:
    RenderContainer() {}
    virtual ~RenderContainer() {}

    void drawSk(SkCanvas *canvas, SkPaint *paint);

    void updatePaintTransformGivenNewCombinedTransform(
            const QMatrix &combinedTransform);

    void setTransform(const QMatrix &transform);

    const QMatrix &getTransform() const;

    const QMatrix &getPaintTransform() const;

    const SkPoint &getDrawpos() const;

    const qreal &getResolutionFraction() const;

    void updateVariables(const QMatrix &combinedTransform,
                         const qreal &effectsMargin,
                         const qreal &resolutionPer,
                         BoundingBox *target);

    void setVariables(const QMatrix &transform,
                      const QMatrix &paintTransform,
                      const SkPoint &drawpos,
                      const sk_sp<SkImage> &imgSk,
                      const qreal &res);
    void duplicateFrom(RenderContainer *src);
    void setVariablesFromRenderData(BoundingBoxRenderData *data);
protected:
    qreal mResolutionFraction;
    QMatrix mTransform;
    QMatrix mPaintTransform;
    SkPoint mDrawPos;
};

#endif // BOUNDINGBOXRENDERCONTAINER_H
