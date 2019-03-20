#include "boundingboxrendercontainer.h"
#include "boundingbox.h"
#include <QElapsedTimer>
#include "memoryhandler.h"
#include "rendercachehandler.h"
#include "GUI/mainwindow.h"
#include "canvas.h"
#include "skia/skiahelpers.h"

RenderContainer::~RenderContainer() {}

void RenderContainer::drawSk(SkCanvas *canvas, SkPaint *paint,
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
            path.addRect(SkRect::MakeXYWH(mDrawPos.x(), mDrawPos.y(),
                                          mImageSk->width(),
                                          mImageSk->height()));
            path.toggleInverseFillType();
            canvas->drawPath(path, paintT);
        }
    }
    //paint->setAntiAlias(true);
    //paint->setFilterQuality(kHigh_SkFilterQuality);
    SkiaHelpers::drawImageGPU(canvas, mImageSk,
                              mDrawPos.x(), mDrawPos.y(), paint, grContext);
    //canvas->drawImage(mImageSk, mDrawPos.x(), mDrawPos.y(), paint);
    canvas->restore();
}

void RenderContainer::updatePaintTransformGivenNewCombinedTransform(
                                    const QMatrix &combinedTransform) {
    mPaintTransform = mTransform.inverted()*combinedTransform;
    mPaintTransform.scale(1/mResolutionFraction,
                          1/mResolutionFraction);
}

void RenderContainer::setTransform(const QMatrix &transform) {
    mTransform = transform;
}

const QMatrix &RenderContainer::getTransform() const {
    return mTransform;
}

const QMatrix &RenderContainer::getPaintTransform() const {
    return mPaintTransform;
}

const SkPoint &RenderContainer::getDrawpos() const {
    return mDrawPos;
}

const qreal &RenderContainer::getResolutionFraction() const {
    return mResolutionFraction;
}

void RenderContainer::setSrcRenderData(BoundingBoxRenderData *data) {
    mNoDataInMemory = false;
    scheduleDeleteTmpFile();

    mTransform = data->fTransform;
    mResolutionFraction = data->fResolution;
    mImageSk = data->fRenderedImage;
    mDrawPos = data->fDrawPos;
    mRelFrame = data->fRelFrame;
    mPaintTransform.reset();
    mPaintTransform.scale(1/mResolutionFraction, 1/mResolutionFraction);
    mSrcRenderData = GetAsSPtr(data, BoundingBoxRenderData);
    MemoryHandler::getInstance()->containerUpdated(this);
}
