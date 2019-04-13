#ifndef BOUNDINGBOXRENDERCONTAINER_H
#define BOUNDINGBOXRENDERCONTAINER_H
class BoundingBox;
#include "smartPointers/sharedpointerdefs.h"
#include "skia/skqtconversions.h"
#include "skia/skiaincludes.h"
#include <QDebug>
#include "smartPointers/sharedpointerdefs.h"
#include "updatable.h"
#include "framerange.h"
#include "CacheHandlers/minimalcachehandler.h"
#include "CacheHandlers/imagecachecontainer.h"
class Canvas;

class RenderCacheHandler;
struct BoundingBoxRenderData;

class RenderContainer {
public:
    RenderContainer() {}

    void drawSk(SkCanvas * const canvas, SkPaint *paint,
                GrContext* const grContext);

    void updatePaintTransformGivenNewTotalTransform(
            const QMatrix &totalTransform);

    void setTransform(const QMatrix &transform);

    const QMatrix &getTransform() const;

    const QMatrix &getPaintTransform() const;

    const SkPoint &getDrawpos() const;

    const qreal &getResolutionFraction() const;

    void setSrcRenderData(BoundingBoxRenderData * const data);
    const int& getRelFrame() const {
        return mRelFrame;
    }

    BoundingBoxRenderData *getSrcRenderData() const {
        return mSrcRenderData.get();
    }

    const bool& isExpired() const {
        return mExpired;
    }

    void setExpired(const bool& expired) {
        mExpired = expired;
    }
protected:
    bool mExpired = false;
    int mRelFrame = 0;
    qreal mResolutionFraction;
    SkPoint mDrawPos;
    QMatrix mTransform;
    QMatrix mPaintTransform;
    stdsptr<BoundingBoxRenderData> mSrcRenderData;
    sk_sp<SkImage> mImageSk;
};

#endif // BOUNDINGBOXRENDERCONTAINER_H
