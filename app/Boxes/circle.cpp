#include "Boxes/circle.h"
#include "canvas.h"
#include "MovablePoints/movablepoint.h"
#include "Animators/gradientpoints.h"
#include "PropertyUpdaters/nodepointupdater.h"
#include "Animators/transformanimator.h"
#include "Animators/effectanimators.h"

Circle::Circle() :
    PathBox(TYPE_CIRCLE) {
    setName("Circle");

    mCenterAnimator = SPtrCreate(QPointFAnimator)("center");
    mCenterPoint = SPtrCreate(AnimatedPoint)(mCenterAnimator.get(),
                                             mTransformAnimator.get(),
                                             TYPE_PATH_POINT);

    mCenterPoint->disableSelection();
    mCenterPoint->setRelativePos(QPointF(0, 0));
    mCenterAnimator->prp_setInheritedUpdater(SPtrCreate(NodePointUpdater)(this));
    ca_addChildAnimator(mCenterAnimator);
    ca_prependChildAnimator(mCenterAnimator.get(), mEffectsAnimators);

    mHorizontalRadiusAnimator =
            SPtrCreate(QPointFAnimator)("horizontal radius");
    mHorizontalRadiusPoint =
            SPtrCreate(CircleRadiusPoint)(mHorizontalRadiusAnimator.get(),
                                          mTransformAnimator.get(),
                                          mCenterPoint.get(),
                                          TYPE_PATH_POINT, false);
    mHorizontalRadiusPoint->setRelativePos(QPointF(10, 0));
    QrealAnimator *hXAnimator = mHorizontalRadiusAnimator->getXAnimator();
    ca_addChildAnimator(GetAsSPtr(hXAnimator, QrealAnimator));
    ca_prependChildAnimator(hXAnimator, mEffectsAnimators);
    hXAnimator->prp_setName("horizontal radius");

    mVerticalRadiusAnimator =
            SPtrCreate(QPointFAnimator)("vertical radius");
    mVerticalRadiusPoint =
            SPtrCreate(CircleRadiusPoint)(mVerticalRadiusAnimator.get(),
                                          mTransformAnimator.get(),
                                          mCenterPoint.get(),
                                          TYPE_PATH_POINT, true);
    mVerticalRadiusPoint->setRelativePos(QPointF(0, 10));
    QrealAnimator *vYAnimator = mVerticalRadiusAnimator->getYAnimator();
    ca_addChildAnimator(GetAsSPtr(vYAnimator, QrealAnimator));
    ca_prependChildAnimator(vYAnimator, mEffectsAnimators);
    vYAnimator->prp_setName("vertical radius");
    prp_setInheritedUpdater(SPtrCreate(NodePointUpdater)(this));
}

void Circle::startAllPointsTransform() {
    mVerticalRadiusPoint->startTransform();
    mHorizontalRadiusPoint->startTransform();
}

void Circle::moveRadiusesByAbs(const QPointF &absTrans) {
    mVerticalRadiusPoint->moveByAbs(absTrans);
    mHorizontalRadiusPoint->moveByAbs(absTrans);
}

void Circle::setVerticalRadius(const qreal &verticalRadius) {
    const QPointF centerPos = mCenterPoint->getRelativePos();
    mVerticalRadiusPoint->setRelativePos(
                centerPos + QPointF(0, verticalRadius));
}

void Circle::setHorizontalRadius(const qreal &horizontalRadius) {
    const QPointF centerPos = mCenterPoint->getRelativePos();
    mHorizontalRadiusPoint->setRelativePos(
                centerPos + QPointF(horizontalRadius, 0));
}

void Circle::setRadius(const qreal &radius) {
    setHorizontalRadius(radius);
    setVerticalRadius(radius);
}

bool Circle::SWT_isCircle() const { return true; }

SkPath Circle::getPathAtRelFrameF(const qreal &relFrame) {
    const SkScalar xRadius = static_cast<SkScalar>(
                mHorizontalRadiusAnimator->getEffectiveXValue(relFrame));
    const SkScalar yRadius = static_cast<SkScalar>(
                mVerticalRadiusAnimator->getEffectiveYValue(relFrame));
    SkRect rect = SkRect::MakeXYWH(-xRadius, -yRadius, 2*xRadius, 2*yRadius);
    const QPointF center = mCenterAnimator->getEffectiveValue();
    rect.offset(toSkPoint(center));
    SkPath path;
    path.addOval(rect);
    return path;
}

qreal Circle::getCurrentXRadius() {
    return mHorizontalRadiusAnimator->getEffectiveXValue();
}

qreal Circle::getCurrentYRadius() {
    return mVerticalRadiusAnimator->getEffectiveYValue();
}

void Circle::getMotionBlurProperties(QList<Property*> &list) const {
    PathBox::getMotionBlurProperties(list);
    list.append(mHorizontalRadiusAnimator.get());
    list.append(mVerticalRadiusAnimator.get());
}

bool Circle::differenceInEditPathBetweenFrames(
        const int& frame1, const int& frame2) const {
    if(mCenterAnimator->prp_differencesBetweenRelFrames(frame1, frame2)) return true;
    if(mHorizontalRadiusAnimator->prp_differencesBetweenRelFrames(frame1, frame2)) return true;
    return mVerticalRadiusAnimator->prp_differencesBetweenRelFrames(frame1, frame2);
}

CircleRadiusPoint::CircleRadiusPoint(QPointFAnimator * const associatedAnimator,
                                     BasicTransformAnimator * const parent,
                                     AnimatedPoint * const centerPoint,
                                     const MovablePointType &type,
                                     const bool &blockX) :
    AnimatedPoint(associatedAnimator, type),
    mXBlocked(blockX), mCenterPoint(centerPoint) {
    setTransform(parent);
    disableSelection();
}

QPointF CircleRadiusPoint::getRelativePos() const {
    const QPointF centerPos = mCenterPoint->getRelativePos();
    return AnimatedPoint::getRelativePos() + centerPos;
}

void CircleRadiusPoint::setRelativePos(const QPointF &relPos) {
    const QPointF centerPos = mCenterPoint->getRelativePos();
    if(mXBlocked) {
        setValue(QPointF(0, relPos.y() - centerPos.y()));
    } else {
        setValue(QPointF(relPos.x() - centerPos.x(), 0));
    }
}
