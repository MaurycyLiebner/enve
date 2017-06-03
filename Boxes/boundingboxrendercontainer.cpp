#include "boundingboxrendercontainer.h"
#include "boundingbox.h"
#include <QElapsedTimer>
#include "memoryhandler.h"
#include "rendercachehandler.h"

void RenderContainer::draw(QPainter *p) {
    p->save();
    p->setTransform(QTransform(mPaintTransform), true);
    p->drawImage(mDrawPos, mImage);
    p->restore();
}

void RenderContainer::drawSk(SkCanvas *canvas, SkPaint *paint) {
    canvas->save();
    canvas->concat(QMatrixToSkMatrix(mPaintTransform));
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

void RenderContainer::setDrawPos(const QPoint &drawpos) {
    mDrawPos = drawpos;
}

const QMatrix &RenderContainer::getTransform() const {
    return mTransform;
}

const QImage &RenderContainer::getImage() const {
    return mImage;
}

const QMatrix &RenderContainer::getPaintTransform() const {
    return mPaintTransform;
}

const QPoint &RenderContainer::getDrawpos() const {
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
    replaceImage(target->getAllUglyPixmapProvidedTransform(
                 resolutionPer*effectsMargin,
                 resolutionPer,
                 mTransform,
                 &mDrawPos));
    updatePaintTransformGivenNewCombinedTransform(combinedTransform);

    target->applyEffects(&mImage, resolutionPer);

    // SKIA

    mImageSk = target->getAllUglyPixmapProvidedTransformSk(
                            resolutionPer*effectsMargin,
                            resolutionPer,
                            mTransform);

    //mRenderTime = timer.elapsed();
}

void RenderContainer::duplicateFrom(RenderContainer *src) {
    setVariables(src->getTransform(),
                 src->getPaintTransform(),
                 src->getDrawpos(),
                 src->getImage(),
                 src->getImageSk(),
                 src->getResolutionFraction());
}

void RenderContainer::setVariables(const QMatrix &transform,
                                   const QMatrix &paintTransform,
                                   const QPoint &drawpos,
                                   const QImage &img,
                                   const sk_sp<SkImage> &imgSk,
                                   const qreal &res) {
    mTransform = transform;
    mPaintTransform = paintTransform;
    replaceImage(img);
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

void CacheContainer::replaceImage(const QImage &img) {
    //MemoryChecker::getInstance()->decUsedMemory(mImage.byteCount());
    mImage = img;
    //MemoryChecker::getInstance()->incUsedMemory(mImage.byteCount());
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

void CacheContainer::draw(QPainter *p) {
    p->drawImage(0, 0, mImage);
}

void CacheContainer::drawSk(SkCanvas *canvas) {
    canvas->drawImage(mImageSk, 0, 0);
}
