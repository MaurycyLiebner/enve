#include "boundingboxrendercontainer.h"
#include "boundingbox.h"
#include <QElapsedTimer>
#include "memoryhandler.h"
#include "GUI/mainwindow.h"
#include "canvas.h"
#include "skia/skiahelpers.h"

void RenderContainer::drawSk(SkCanvas * const canvas,
                             SkPaint * const paint,
                             GrContext * const grContext) {
    if(!mSrcRenderData) return;
    canvas->save();
    canvas->concat(toSkMatrix(mPaintTransform));
    if(paint) {
        if(paint->getBlendMode() == SkBlendMode::kDstIn ||
           paint->getBlendMode() == SkBlendMode::kSrcIn ||
           paint->getBlendMode() == SkBlendMode::kDstATop) {
            SkPaint paintT;
            paintT.setBlendMode(paint->getBlendMode());
            paintT.setColor(SK_ColorTRANSPARENT);
            SkPath path;
            path.addRect(SkRect::MakeXYWH(mGlobalRect.x(), mGlobalRect.y(),
                                          mImageSk->width(),
                                          mImageSk->height()));
            path.toggleInverseFillType();
            canvas->drawPath(path, paintT);
        }
    }
    //paint->setAntiAlias(true);
    //paint->setFilterQuality(kHigh_SkFilterQuality);
    SkiaHelpers::drawImageGPU(canvas, mImageSk,
                              mGlobalRect.x(), mGlobalRect.y(), paint, grContext);
    //canvas->drawImage(mImageSk, mDrawPos.x(), mDrawPos.y(), paint);
    canvas->restore();
}

void RenderContainer::updatePaintTransformGivenNewTotalTransform(
                                    const QMatrix &totalTransform) {
    mPaintTransform = mTransform.inverted()*totalTransform;
    const qreal invRes = 1/mResolutionFraction;
    mPaintTransform.scale(invRes, invRes);
    mPaintTransform = mRenderTransform*mPaintTransform;
}

void RenderContainer::setSrcRenderData(BoundingBoxRenderData * const data) {
    mTransform = data->fTransform;
    mResolutionFraction = data->fResolution;
    mImageSk = data->fRenderedImage;
    mGlobalRect = data->fGlobalRect;
    mRelFrame = data->fRelFrame;
    mPaintTransform.reset();
    mPaintTransform.scale(1/mResolutionFraction, 1/mResolutionFraction);
    mRenderTransform = data->fRenderTransform;
    mPaintTransform = mRenderTransform*mPaintTransform;
    mSrcRenderData = GetAsSPtr(data, BoundingBoxRenderData);
}
