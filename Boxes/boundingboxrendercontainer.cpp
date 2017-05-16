#include "boundingboxrendercontainer.h"
#include "boundingbox.h"
#include <QElapsedTimer>
#include "memoryhandler.h"

void RenderContainer::draw(QPainter *p) {
    p->save();
    p->setTransform(QTransform(mPaintTransform), true);
    p->drawImage(mBoundingRect.topLeft(), mImage);
    p->restore();
}

void RenderContainer::updatePaintTransformGivenNewCombinedTransform(
                                    const QMatrix &combinedTransform) {
    mPaintTransform = mTransform.inverted()*combinedTransform;
}

void RenderContainer::replaceImage(const QImage &img) {
    mImage = img;
}

void RenderContainer::setTransform(const QMatrix &transform) {
    mTransform = transform;
}

void RenderContainer::setBoundingRect(const QRectF &rect) {
    mBoundingRect = rect;
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

const QRectF &RenderContainer::getBoundingRect() const {
    return mBoundingRect;
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
    mImage = target->getAllUglyPixmapProvidedTransform(
                resolutionPer*effectsMargin,
                resolutionPer,
                mTransform,
                &mBoundingRect);
    updatePaintTransformGivenNewCombinedTransform(combinedTransform);

    target->applyEffects(&mImage,
                         resolutionPer);

    //mRenderTime = timer.elapsed();
}

void RenderContainer::duplicateFrom(RenderContainer *src) {
    setVariables(src->getTransform(),
                 src->getPaintTransform(),
                 src->getBoundingRect(),
                 src->getImage(),
                 src->getResolutionFraction());
}

void RenderContainer::setVariables(const QMatrix &transform,
                                   const QMatrix &paintTransform,
                                   const QRectF &rect,
                                   const QImage &img,
                                   const qreal &res) {
    mTransform = transform;
    mPaintTransform = paintTransform;
    mImage = img;
    mBoundingRect = rect;
    mResolutionFraction = res;
    thisAccessed();
}

CacheContainer::CacheContainer() {
    MemoryHandler::getInstance()->addContainer(this);
}

CacheContainer::~CacheContainer() {
    MemoryHandler::getInstance()->removeContainer(this);
}

void CacheContainer::setParentCacheHandler(CacheHandler *handler) {
    mParentCacheHandler = handler;
}

void CacheContainer::freeThis() {
    if(mParentCacheHandler == NULL) return;
    mParentCacheHandler->removeRenderContainer(this);
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
