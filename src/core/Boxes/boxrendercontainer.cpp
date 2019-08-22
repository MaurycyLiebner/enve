#include "boxrendercontainer.h"
#include "boxrenderdata.h"
#include "skia/skiahelpers.h"

void RenderContainer::drawSk(SkCanvas * const canvas, SkPaint * const paint) {
    if(!mSrcRenderData) return;
    canvas->save();
    canvas->concat(toSkMatrix(mPaintTransform));
    if(paint) {
        const auto blendMode = paint->getBlendMode();
        if(blendMode == SkBlendMode::kDstIn ||
           blendMode == SkBlendMode::kSrcIn ||
           blendMode == SkBlendMode::kDstATop) {
            SkPaint bPaint;
            bPaint.setBlendMode(paint->getBlendMode());
            bPaint.setColor(SK_ColorTRANSPARENT);
            SkPath path;
            path.addRect(SkRect::MakeXYWH(mGlobalRect.x(), mGlobalRect.y(),
                                          mImageSk->width(),
                                          mImageSk->height()));
            path.toggleInverseFillType();
            canvas->drawPath(path, bPaint);
        }
    }
    canvas->drawImage(mImageSk, mGlobalRect.x(), mGlobalRect.y(), paint);
    canvas->restore();
}

void RenderContainer::updatePaintTransformGivenNewTotalTransform(
                                    const QMatrix &totalTransform) {
    mPaintTransform = mTransform.inverted()*totalTransform;
    const qreal invRes = 1/mResolutionFraction;
    mPaintTransform.scale(invRes, invRes);
    mPaintTransform = mRenderTransform*mPaintTransform;
}

void RenderContainer::setSrcRenderData(BoxRenderData * const data) {
    mTransform = data->fTransform;
    mResolutionFraction = data->fResolution;
    mImageSk = data->fRenderedImage;
    mGlobalRect = data->fGlobalRect;
    mRelFrame = data->fRelFrame;
    mPaintTransform.reset();
    mPaintTransform.scale(1/mResolutionFraction, 1/mResolutionFraction);
    mRenderTransform = data->fRenderTransform;
    mPaintTransform = mRenderTransform*mPaintTransform;
    mSrcRenderData = data->ref<BoxRenderData>();
}
