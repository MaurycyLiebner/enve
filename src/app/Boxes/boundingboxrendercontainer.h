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
#include "CacheHandlers/imagecachecontainer.h"
class Canvas;

class HDDCachableCacheHandler;
struct BoundingBoxRenderData;

class RenderContainer {
public:
    RenderContainer() {}

    void drawSk(SkCanvas * const canvas,
                SkPaint * const paint,
                GrContext* const grContext);

    void updatePaintTransformGivenNewTotalTransform(
            const QMatrix &totalTransform);

    void setSrcRenderData(BoundingBoxRenderData * const data);
    int getRelFrame() const {
        return mRelFrame;
    }

    BoundingBoxRenderData *getSrcRenderData() const {
        return mSrcRenderData.get();
    }

    bool isExpired() const {
        return mExpired;
    }

    void setExpired(const bool expired) {
        mExpired = expired;
    }
protected:
    bool mExpired = false;
    int mRelFrame = 0;
    qreal mResolutionFraction;
    QRect mGlobalRect;
    QMatrix mTransform;
    QMatrix mPaintTransform;
    QMatrix mRenderTransform;
    stdsptr<BoundingBoxRenderData> mSrcRenderData;
    sk_sp<SkImage> mImageSk;
};

#endif // BOUNDINGBOXRENDERCONTAINER_H
