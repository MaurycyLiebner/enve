#ifndef RENDERCONTAINER_H
#define RENDERCONTAINER_H
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
struct BoxRenderData;

class RenderContainer {
public:
    RenderContainer() {}

    void drawSk(SkCanvas * const canvas,
                SkPaint * const paint,
                GrContext* const grContext);

    void updatePaintTransformGivenNewTotalTransform(
            const QMatrix &totalTransform);

    void setSrcRenderData(BoxRenderData * const data);
    int getRelFrame() const {
        return mRelFrame;
    }

    BoxRenderData *getSrcRenderData() const {
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
    stdsptr<BoxRenderData> mSrcRenderData;
    sk_sp<SkImage> mImageSk;
};

#endif // RENDERCONTAINER_H
