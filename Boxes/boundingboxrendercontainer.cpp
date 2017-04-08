#include "boundingboxrendercontainer.h"
#include "boundingbox.h"
#include <QElapsedTimer>
#include "memoryhandler.h"

void BoundingBoxRenderContainer::draw(QPainter *p) {
    p->save();
    p->setTransform(QTransform(mPaintTransform), true);
    p->drawImage(mBoundingRect.topLeft(), mImage);
    p->restore();
}

void BoundingBoxRenderContainer::drawWithoutTransform(QPainter *p) {
    draw(p); return;
    //p->setTransform(QTransform(mPaintTransform), true);
    p->drawImage(0, 0, mImage);
}

void BoundingBoxRenderContainer::updatePaintTransformGivenNewCombinedTransform(
                                    const QMatrix &combinedTransform) {
    mPaintTransform = mTransform.inverted()*combinedTransform;
}

void BoundingBoxRenderContainer::replaceImage(const QImage &img) {
    mImage = img;
}

void BoundingBoxRenderContainer::setTransform(const QMatrix &transform) {
    mTransform = transform;
}

void BoundingBoxRenderContainer::setBoundingRect(const QRectF &rect) {
    mBoundingRect = rect;
}

const QMatrix &BoundingBoxRenderContainer::getTransform() const {
    return mTransform;
}

const QImage &BoundingBoxRenderContainer::getImage() const {
    return mImage;
}

const QMatrix &BoundingBoxRenderContainer::getPaintTransform() const {
    return mPaintTransform;
}

const QRectF &BoundingBoxRenderContainer::getBoundingRect() const {
    return mBoundingRect;
}

const qreal &BoundingBoxRenderContainer::getResolutionPercent() const {
    return mResolutionPercent;
}

const int &BoundingBoxRenderContainer::getFrame() const {
    return mFrame;
}

const qint64 &BoundingBoxRenderContainer::getRenderTime() const {
    return mRenderTime;
}

void BoundingBoxRenderContainer::setRelFrame(const int &frame) {
    mFrame = frame;
}

void BoundingBoxRenderContainer::updateVariables(const QMatrix &combinedTransform,
                                                 const qreal &effectsMargin,
                                                 const qreal &resolutionPer,
                                                 BoundingBox *target) {
    QElapsedTimer timer;
    timer.start();

    mTransform = combinedTransform;
    mTransform.scale(resolutionPer, resolutionPer);

    mResolutionPercent = resolutionPer;
    mImage = target->getAllUglyPixmapProvidedTransform(
                resolutionPer*effectsMargin,
                mTransform,
                &mBoundingRect);
    updatePaintTransformGivenNewCombinedTransform(combinedTransform);

    target->applyEffects(&mImage,
                         resolutionPer);

    mRenderTime = timer.elapsed();
}

CacheBoundingBoxRenderContainer::CacheBoundingBoxRenderContainer() {
    MemoryHandler::getInstance()->addContainer(this);
}

CacheBoundingBoxRenderContainer::~CacheBoundingBoxRenderContainer() {
    MemoryHandler::getInstance()->removeContainer(this);
}

void CacheBoundingBoxRenderContainer::duplicateFrom(
                                            BoundingBoxRenderContainer *src) {
    setVariables(src->getTransform(),
                 src->getPaintTransform(),
                 src->getBoundingRect(),
                 src->getImage(),
                 src->getFrame(),
                 src->getResolutionPercent(),
                 src->getRenderTime());
}

void CacheBoundingBoxRenderContainer::setVariables(const QMatrix &transform,
                                              const QMatrix &paintTransform,
                                              const QRectF &rect,
                                              const QImage &img,
                                              const int &frame,
                                              const qreal &res,
                                              const qint64 &time) {
    mTransform = transform;
    mPaintTransform = paintTransform;
    mImage = img;
    mBoundingRect = rect;
    mFrame = frame;
    mResolutionPercent = res;
    mRenderTime = time;
    thisAccessed();
}

void CacheBoundingBoxRenderContainer::thisAccessed() {
    MemoryHandler::getInstance()->containerUpdated(this);
}

void CacheBoundingBoxRenderContainer::freeThis() {
    if(mParentRange == NULL) return;
    mParentRange->removeRenderContainer(this);
}
