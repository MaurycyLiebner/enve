#include "boundingboxrendercontainer.h"
#include "boundingbox.h"
#include <QElapsedTimer>
#include "memoryhandler.h"
#include "rendercachehandler.h"

void RenderContainer::drawSk(SkCanvas *canvas, SkPaint *paint) {
    canvas->save();
    canvas->concat(QMatrixToSkMatrix(mPaintTransform));
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
    canvas->drawImage(mImageSk, mDrawPos.x(), mDrawPos.y(), paint);
    canvas->restore();
}

void RenderContainer::updatePaintTransformGivenNewCombinedTransform(
                                    const QMatrix &combinedTransform) {
    mPaintTransform = mTransform.inverted()*combinedTransform;
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

void RenderContainer::updateVariables(const QMatrix &combinedTransform,
                                      const qreal &effectsMargin,
                                      const qreal &resolutionPer,
                                      BoundingBox *target) {
    //QElapsedTimer timer;
    //timer.start();

    mTransform = combinedTransform;
    mTransform.scale(resolutionPer, resolutionPer);

    mResolutionFraction = resolutionPer;

    // SKIA

    mImageSk = target->getAllUglyPixmapProvidedTransformSk(
                            resolutionPer*effectsMargin,
                            resolutionPer,
                            mTransform,
                            &mDrawPos);
    updatePaintTransformGivenNewCombinedTransform(combinedTransform);

    //mRenderTime = timer.elapsed();
}

void RenderContainer::duplicateFrom(RenderContainer *src) {
    setVariables(src->getTransform(),
                 src->getPaintTransform(),
                 src->getDrawpos(),
                 src->getImageSk(),
                 src->getResolutionFraction());
}

void RenderContainer::setVariables(const QMatrix &transform,
                                   const QMatrix &paintTransform,
                                   const SkPoint &drawpos,
                                   const sk_sp<SkImage> &imgSk,
                                   const qreal &res) {
    mTransform = transform;
    mPaintTransform = paintTransform;
    mImageSk = imgSk;
    mDrawPos = drawpos;
    mResolutionFraction = res;
    thisAccessed();
}

CacheContainer::CacheContainer() {
    MemoryHandler::getInstance()->addContainer(this);
}

CacheContainer::~CacheContainer() {
    //MemoryChecker::getInstance()->decUsedMemory(mImage.byteCount());
    MemoryHandler::getInstance()->removeContainer(this);
}

void CacheContainer::setParentCacheHandler(CacheHandler *handler) {
    mParentCacheHandler = handler;
}


void CacheContainer::replaceImageSk(const sk_sp<SkImage> &img) {
    mImageSk = img;
}

bool CacheContainer::freeThis() {
    if(mBlocked) return false;
    if(mParentCacheHandler == NULL) return false;
    mParentCacheHandler->removeRenderContainer(this);
    return true;
}

void CacheContainer::thisAccessed() {
    MemoryHandler::getInstance()->containerUpdated(this);
}

const int &CacheContainer::getMinRelFrame() const {
    return mMinRelFrame;
}

const int &CacheContainer::getMaxRelFrame() const {
    return mMaxRelFrame;
}

bool CacheContainer::relFrameInRange(const int &relFrame) {
    return relFrame >= mMinRelFrame && relFrame < mMaxRelFrame;
}

void CacheContainer::setRelFrame(const int &frame) {
    mMinRelFrame = frame;
    mMaxRelFrame = frame + 1;
}

void CacheContainer::setMaxRelFrame(const int &maxFrame) {
    mMaxRelFrame = maxFrame;
}

void CacheContainer::setMinRelFrame(const int &minFrame) {
    mMinRelFrame = minFrame;
}

void CacheContainer::setRelFrameRange(const int &minFrame,
                                      const int &maxFrame) {
    mMinRelFrame = minFrame;
    mMaxRelFrame = maxFrame;
}

void CacheContainer::drawSk(SkCanvas *canvas) {
    canvas->drawImage(mImageSk, 0, 0);
}
