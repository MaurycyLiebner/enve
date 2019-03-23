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
    mCenterPoint = SPtrCreate(CircleCenterPoint)(mCenterAnimator.get(),
                                                 mTransformAnimator.get(),
                                                 TYPE_PATH_POINT);
    mCenterPoint->setRelativePos(QPointF(0., 0.));

    mHorizontalRadiusAnimator =
            SPtrCreate(QPointFAnimator)("horizontal radius");
    mHorizontalRadiusPoint =
            SPtrCreate(CircleRadiusPoint)(mHorizontalRadiusAnimator.get(),
                                          mTransformAnimator.get(),
                                          TYPE_PATH_POINT,
                                          false, mCenterPoint.get());
    mHorizontalRadiusPoint->setRelativePos(QPointF(10., 0.));

    mVerticalRadiusAnimator =
            SPtrCreate(QPointFAnimator)("vertical radius");
    mVerticalRadiusPoint =
            SPtrCreate(CircleRadiusPoint)(mVerticalRadiusAnimator.get(),
                                          mTransformAnimator.get(),
                                          TYPE_PATH_POINT,
                                          true, mCenterPoint.get());
    mVerticalRadiusPoint->setRelativePos(QPointF(0., 10.));

    QrealAnimator *hXAnimator = mHorizontalRadiusAnimator->getXAnimator();
    ca_addChildAnimator(GetAsSPtr(hXAnimator, QrealAnimator));
    ca_prependChildAnimator(hXAnimator, mEffectsAnimators);
    hXAnimator->prp_setName("horizontal radius");

    QrealAnimator *vYAnimator = mVerticalRadiusAnimator->getYAnimator();
    ca_addChildAnimator(GetAsSPtr(vYAnimator, QrealAnimator));
    ca_prependChildAnimator(vYAnimator, mEffectsAnimators);
    vYAnimator->prp_setName("vertical radius");


    mCenterPoint->setVerticalAndHorizontalPoints(
                mVerticalRadiusPoint.get(),
                mHorizontalRadiusPoint.get());

    prp_setInheritedUpdater(SPtrCreate(NodePointUpdater)(this));
    mCenterAnimator->prp_setInheritedUpdater(SPtrCreate(NodePointUpdater)(this));
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
    mVerticalRadiusPoint->setRelativePos(QPointF(0., verticalRadius) );
}

void Circle::setHorizontalRadius(const qreal &horizontalRadius)
{
    mHorizontalRadiusPoint->setRelativePos(QPointF(horizontalRadius, 0.) );
}

void Circle::setRadius(const qreal &radius) {
    setHorizontalRadius(radius);
    setVerticalRadius(radius);
}

void Circle::drawCanvasControls(SkCanvas * const canvas,
                            const CanvasMode &currentCanvasMode,
                            const SkScalar &invScale) {
    BoundingBox::drawCanvasControls(canvas, currentCanvasMode, invScale);
    if(currentCanvasMode == CanvasMode::MOVE_POINT) {
        mCenterPoint->drawSk(canvas, invScale);
        mHorizontalRadiusPoint->drawSk(canvas, invScale);
        mVerticalRadiusPoint->drawSk(canvas, invScale);

        mFillGradientPoints->drawGradientPointsSk(canvas, invScale);
        mStrokeGradientPoints->drawGradientPointsSk(canvas, invScale);
    }
}

bool Circle::SWT_isCircle() const { return true; }

MovablePoint *Circle::getPointAtAbsPos(
        const QPointF &absPtPos,
                                const CanvasMode &currentCanvasMode,
                                const qreal &canvasScaleInv) {
    MovablePoint* pointToReturn = PathBox::getPointAtAbsPos(absPtPos,
                                                            currentCanvasMode,
                                                            canvasScaleInv);
    if(!pointToReturn) {
        if(mHorizontalRadiusPoint->isPointAtAbsPos(absPtPos, canvasScaleInv) ) {
            return mHorizontalRadiusPoint.get();
        }
        if(mVerticalRadiusPoint->isPointAtAbsPos(absPtPos, canvasScaleInv) ) {
            return mVerticalRadiusPoint.get();
        }
        if(mCenterPoint->isPointAtAbsPos(absPtPos, canvasScaleInv)) {
            return mCenterPoint.get();
        }
    }
    return pointToReturn;
}

void Circle::selectAndAddContainedPointsToList(const QRectF &absRect,
                                               QList<stdptr<MovablePoint>> &list) {
    if(!mCenterPoint->isSelected()) {
        if(mCenterPoint->isContainedInRect(absRect)) {
            mCenterPoint->select();
            list.append(mCenterPoint.get());
        }
    }
    if(!mHorizontalRadiusPoint->isSelected()) {
        if(mHorizontalRadiusPoint->isContainedInRect(absRect)) {
            mHorizontalRadiusPoint->select();
            list.append(mHorizontalRadiusPoint.get());
        }
    }
    if(!mVerticalRadiusPoint->isSelected()) {
        if(mVerticalRadiusPoint->isContainedInRect(absRect)) {
            mVerticalRadiusPoint->select();
            list.append(mVerticalRadiusPoint.get());
        }
    }
}

SkPath Circle::getPathAtRelFrameF(const qreal &relFrame) {
    SkScalar xRadius = static_cast<SkScalar>(
                mHorizontalRadiusAnimator->getEffectiveXValueAtRelFrame(relFrame));
    SkScalar yRadius = static_cast<SkScalar>(
                mVerticalRadiusAnimator->getEffectiveYValueAtRelFrame(relFrame));
    SkPath path;
    path.addOval(SkRect::MakeXYWH(-xRadius, -yRadius,
                                  2*xRadius, 2*yRadius));
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

CircleCenterPoint::CircleCenterPoint(QPointFAnimator* associatedAnimator,
                                     BasicTransformAnimator* parent,
                                     const MovablePointType& type) :
    AnimatedPoint(associatedAnimator, parent, type) {}

void CircleCenterPoint::setVerticalAndHorizontalPoints(
        MovablePoint* verticalPoint,
        MovablePoint* horizontalPoint) {
    mVerticalPoint_cv = verticalPoint;
    mHorizontalPoint_cv = horizontalPoint;
}

void CircleCenterPoint::moveByRel(const QPointF &relTranslatione) {
    mParentTransform_cv->moveRelativeToSavedValue(relTranslatione.x(),
                                      relTranslatione.y());
}

void CircleCenterPoint::moveByAbs(const QPointF &absTranslatione) {
    mParentTransform_cv->moveByAbs(absTranslatione);
}

void CircleCenterPoint::startTransform() {
    mParentTransform_cv->prp_startTransform();
}

void CircleCenterPoint::finishTransform() {
    mParentTransform_cv->prp_finishTransform();
}

CircleRadiusPoint::CircleRadiusPoint(QPointFAnimator *associatedAnimator,
                                     BasicTransformAnimator* parent,
                                     const MovablePointType &type,
                                     const bool &blockX,
                                     MovablePoint *centerPoint) :
    AnimatedPoint(associatedAnimator, parent, type) {
    mCenterPoint_cv = centerPoint;
    mXBlocked = blockX;
}

void CircleRadiusPoint::moveByRel(const QPointF &relTranslation) {
    if(mCenterPoint_cv->isSelected() ) return;
    QPointF relTranslationT = relTranslation;
    if(mXBlocked) {
        relTranslationT.setX(0.);
    } else {
        relTranslationT.setY(0.);
    }
    AnimatedPoint::moveByRel(relTranslationT);
}

//void CircleRadiusPoint::setAbsPosRadius(QPointF pos)
//{
//    QMatrix combinedM = mParent->getCombinedTransform();
//    QPointF newPos = combinedM.inverted().map(pos);
//    if(mXBlocked) {
//        newPos.setX(getSavedXValue());
//    } else {
//        newPos.setY(getSavedYValue());
//    }
//    setRelativePos(newPos, false );
//}

void CircleRadiusPoint::moveByAbs(const QPointF &absTranslatione) {
    if(mCenterPoint_cv->isSelected() ) return;
    MovablePoint::moveByAbs(absTranslatione);
    //setCurrentValue(mSavedRelPos);
    //setAbsPosRadius(getAbsolutePos() + absTranslatione);
}

void CircleRadiusPoint::setRelativePos(const QPointF &relPos) {
    if(mXBlocked) {
        mAssociatedAnimator_k->getYAnimator()->setCurrentBaseValue(relPos.y());
    } else {
        mAssociatedAnimator_k->getXAnimator()->setCurrentBaseValue(relPos.x());
    }
}

void CircleRadiusPoint::startTransform() {
    if(mCenterPoint_cv->isSelected() ) return;
    AnimatedPoint::startTransform();
}

void CircleRadiusPoint::finishTransform() {
    if(mCenterPoint_cv->isSelected() ) return;
    AnimatedPoint::finishTransform();
}
