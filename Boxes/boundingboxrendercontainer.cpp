#include "boundingboxrendercontainer.h"
#include "boundingbox.h"

BoundingBoxRenderContainer::BoundingBoxRenderContainer() {}

void BoundingBoxRenderContainer::draw(QPainter *p) {
    p->save();
    p->setTransform(QTransform(mPaintTransform), true);
    p->drawImage(mBoundingRect.topLeft(), mImage);
    p->restore();
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

void BoundingBoxRenderContainer::updateVariables(const QMatrix &combinedTransform,
                                                 const QMatrix &transform,
                                                 const qreal &effectsMargin,
                                                 const qreal &resolutionPer,
                                                 BoundingBox *target) {
    mTransform = transform;

    mResolutionPercent = resolutionPer;
    mImage = target->getAllUglyPixmapProvidedTransform(
                resolutionPer*effectsMargin,
                mTransform,
                &mBoundingRect);
    updatePaintTransformGivenNewCombinedTransform(combinedTransform);

    target->applyEffects(&mImage,
                         resolutionPer);
}

void BoundingBoxRenderContainer::duplicateFrom(BoundingBoxRenderContainer *src) {
    setVariables(src->getTransform(),
                 src->getPaintTransform(),
                 src->getBoundingRect(),
                 src->getImage(),
                 src->getFrame(),
                 src->getResolutionPercent());
}

void BoundingBoxRenderContainer::setVariables(const QMatrix &transform,
                                              const QMatrix &paintTransform,
                                              const QRectF &rect,
                                              const QImage &img,
                                              const int &frame,
                                              const qreal &res) {
    mTransform = transform;
    mPaintTransform = paintTransform;
    mImage = img;
    mBoundingRect = rect;
    mFrame = frame;
    mResolutionPercent = res;
}
