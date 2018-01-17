#include "Boxes/circle.h"
#include "canvas.h"
#include "movablepoint.h"
#include "updatescheduler.h"
#include "Animators/animatorupdater.h"
#include "gradientpoints.h"

Circle::Circle() :
    PathBox(TYPE_CIRCLE) {
    setName("Circle");

    mCenter = new CircleCenterPoint(mTransformAnimator.data(), TYPE_PATH_POINT);
    mCenter->setRelativePos(QPointF(0., 0.));
    mHorizontalRadiusPoint = new CircleRadiusPoint(mTransformAnimator.data(),
                                                   TYPE_PATH_POINT,
                                                   false, mCenter);
    mHorizontalRadiusPoint->setRelativePos(QPointF(10., 0.));
    mVerticalRadiusPoint = new CircleRadiusPoint(mTransformAnimator.data(),
                                                 TYPE_PATH_POINT,
                                                 true, mCenter);
    mVerticalRadiusPoint->setRelativePos(QPointF(0., 10.));

    QrealAnimator *hXAnimator = mHorizontalRadiusPoint->getXAnimator();
    ca_prependChildAnimator(hXAnimator, mEffectsAnimators.data());
    hXAnimator->prp_setName("horizontal radius");

    QrealAnimator *vYAnimator = mVerticalRadiusPoint->getYAnimator();
    ca_prependChildAnimator(vYAnimator, mEffectsAnimators.data());
    vYAnimator->prp_setName("vertical radius");


    mCenter->setVerticalAndHorizontalPoints(mVerticalRadiusPoint,
                                            mHorizontalRadiusPoint);

    mCenter->prp_setUpdater(new NodePointUpdater(this) );
    mHorizontalRadiusPoint->prp_setUpdater(new NodePointUpdater(this));
    mVerticalRadiusPoint->prp_setUpdater(new NodePointUpdater(this));
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

void Circle::drawSelectedSk(SkCanvas *canvas,
                            const CanvasMode &currentCanvasMode,
                            const SkScalar &invScale) {
    if(isVisibleAndInVisibleDurationRect()) {
        canvas->save();
        drawBoundingRectSk(canvas, invScale);
        if(currentCanvasMode == CanvasMode::MOVE_POINT) {
            mCenter->drawSk(canvas, invScale);
            mHorizontalRadiusPoint->drawSk(canvas, invScale);
            mVerticalRadiusPoint->drawSk(canvas, invScale);

            mFillGradientPoints->drawGradientPointsSk(canvas, invScale);
            mStrokeGradientPoints->drawGradientPointsSk(canvas, invScale);
        } else if(currentCanvasMode == MOVE_PATH) {
            mTransformAnimator->getPivotMovablePoint()->
                    drawSk(canvas, invScale);
        }
        canvas->restore();
    }
}

MovablePoint *Circle::getPointAtAbsPos(
                                const QPointF &absPtPos,
                                const CanvasMode &currentCanvasMode,
                                const qreal &canvasScaleInv) {
    MovablePoint *pointToReturn = PathBox::getPointAtAbsPos(absPtPos,
                                                            currentCanvasMode,
                                                            canvasScaleInv);
    if(pointToReturn == NULL) {
        if(mHorizontalRadiusPoint->isPointAtAbsPos(absPtPos, canvasScaleInv) ) {
            return mHorizontalRadiusPoint;
        }
        if(mVerticalRadiusPoint->isPointAtAbsPos(absPtPos, canvasScaleInv) ) {
            return mVerticalRadiusPoint;
        }
        if(mCenter->isPointAtAbsPos(absPtPos, canvasScaleInv)) {
            return mCenter;
        }
    }
    return pointToReturn;
}

void Circle::selectAndAddContainedPointsToList(const QRectF &absRect,
                                               QList<MovablePoint *> *list) {
    if(!mCenter->isSelected()) {
        if(mCenter->isContainedInRect(absRect)) {
            mCenter->select();
            list->append(mCenter);
        }
    }
    if(!mHorizontalRadiusPoint->isSelected()) {
        if(mHorizontalRadiusPoint->isContainedInRect(absRect)) {
            mHorizontalRadiusPoint->select();
            list->append(mHorizontalRadiusPoint);
        }
    }
    if(!mVerticalRadiusPoint->isSelected()) {
        if(mVerticalRadiusPoint->isContainedInRect(absRect)) {
            mVerticalRadiusPoint->select();
            list->append(mVerticalRadiusPoint);
        }
    }
}

SkPath Circle::getPathAtRelFrame(const int &relFrame) {
    SkScalar xRadius = mHorizontalRadiusPoint->getEffectiveXValueAtRelFrame(relFrame);
    SkScalar yRadius = mVerticalRadiusPoint->getEffectiveYValueAtRelFrame(relFrame);
    QPainterPath pathT;
    pathT.addEllipse(QPointF(0., 0.), xRadius, yRadius);
    return QPainterPathToSkPath(pathT);
    SkPath path;
    path.addOval(SkRect::MakeXYWH(-xRadius, -yRadius,
                                  2*xRadius, 2*yRadius));
    return path;
}

CircleCenterPoint::CircleCenterPoint(BasicTransformAnimator *parent,
                                     MovablePointType type) :
    PointAnimator(parent, type) {

}


CircleCenterPoint::~CircleCenterPoint() {

}

void CircleCenterPoint::setVerticalAndHorizontalPoints(
                                                MovablePoint *verticalPoint,
                                                MovablePoint *horizontalPoint) {
    mVerticalPoint = verticalPoint;
    mHorizontalPoint = horizontalPoint;
}

void CircleCenterPoint::moveByRel(const QPointF &relTranslatione) {
    mParent->moveRelativeToSavedValue(relTranslatione.x(),
                                      relTranslatione.y());
}

void CircleCenterPoint::moveByAbs(const QPointF &absTranslatione) {
    mParent->moveByAbs(absTranslatione);
}

void CircleCenterPoint::startTransform() {
    mParent->prp_startTransform();
}

void CircleCenterPoint::finishTransform() {
    mParent->prp_finishTransform();
}

CircleRadiusPoint::CircleRadiusPoint(BasicTransformAnimator *parent,
                                     const MovablePointType &type,
                                     const bool &blockX,
                                     MovablePoint *centerPoint) :
    PointAnimator(parent, type) {
    mCenterPoint = centerPoint;
    mXBlocked = blockX;
}

CircleRadiusPoint::~CircleRadiusPoint() {

}

void CircleRadiusPoint::moveByRel(const QPointF &relTranslation) {
    if(mCenterPoint->isSelected() ) return;
    QPointF relTranslationT = relTranslation;
    if(mXBlocked) {
        relTranslationT.setX(0.);
    } else {
        relTranslationT.setY(0.);
    }
    PointAnimator::moveByRel(relTranslationT);
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
    if(mCenterPoint->isSelected() ) return;
    MovablePoint::moveByAbs(absTranslatione);
    //setCurrentValue(mSavedRelPos);
    //setAbsPosRadius(getAbsolutePos() + absTranslatione);
}

void CircleRadiusPoint::setRelativePos(const QPointF &relPos) {
    if(mXBlocked) {
        mYAnimator->qra_setCurrentValue(relPos.y());
    } else {
        mXAnimator->qra_setCurrentValue(relPos.x());
    }
}

void CircleRadiusPoint::startTransform() {
    if(mCenterPoint->isSelected() ) return;
    MovablePoint::startTransform();
}

void CircleRadiusPoint::finishTransform() {
    if(mCenterPoint->isSelected() ) return;
    MovablePoint::finishTransform();
}
