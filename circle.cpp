#include "circle.h"
#include "canvas.h"
#include "movablepoint.h"
#include "updatescheduler.h"

Circle::Circle(BoxesGroup *parent) : PathBox(parent, TYPE_CIRCLE)
{
    setName("Circle");


    mCenter = new CircleCenterPoint(0., 0., this, TYPE_PATH_POINT);
    mHorizontalRadiusPoint = new CircleRadiusPoint(0., 0.,
                                              this, TYPE_PATH_POINT,
                                              false, mCenter);
    mVerticalRadiusPoint = new CircleRadiusPoint(0., 0.,
                                            this, TYPE_PATH_POINT,
                                            true, mCenter);

    QrealAnimator *hXAnimator = mHorizontalRadiusPoint->
                                    getRelativePosAnimatorPtr()->getXAnimator();
    addActiveAnimator(hXAnimator);
    hXAnimator->setName("horizontal radius");

    QrealAnimator *vYAnimator = mVerticalRadiusPoint->
                                    getRelativePosAnimatorPtr()->getYAnimator();
    addActiveAnimator(vYAnimator);
    vYAnimator->setName("vertical radius");


    mCenter->setVerticalAndHorizontalPoints(mVerticalRadiusPoint, mHorizontalRadiusPoint);

    mCenter->setPosAnimatorUpdater(new PathPointUpdater(this) );
    mHorizontalRadiusPoint->setPosAnimatorUpdater(
                new PathPointUpdater(this));
    mVerticalRadiusPoint->setPosAnimatorUpdater(
                new PathPointUpdater(this));

    schedulePathUpdate();
}

void Circle::updateAfterFrameChanged(int currentFrame)
{
    mVerticalRadiusPoint->updateAfterFrameChanged(currentFrame);
    mHorizontalRadiusPoint->updateAfterFrameChanged(currentFrame);
    PathBox::updateAfterFrameChanged(currentFrame);
}

void Circle::moveRadiusesByAbs(QPointF absTrans) {
    mVerticalRadiusPoint->moveByAbs(absTrans);
    mHorizontalRadiusPoint->moveByAbs(absTrans);
}

void Circle::startPointsTransform() {
    mVerticalRadiusPoint->startTransform();
    mHorizontalRadiusPoint->startTransform();
    startTransform();
}

void Circle::setVerticalRadius(qreal verticalRadius)
{
    mVerticalRadiusPoint->setRelativePos(QPointF(0., verticalRadius) );
}

void Circle::setHorizontalRadius(qreal horizontalRadius)
{
    mHorizontalRadiusPoint->setRelativePos(QPointF(horizontalRadius, 0.) );
}

void Circle::setRadius(qreal radius)
{
    setHorizontalRadius(radius);
    setVerticalRadius(radius);
}

void Circle::drawSelected(QPainter *p, CanvasMode currentCanvasMode)
{
    if(mVisible) {
        p->save();
        drawBoundingRect(p);
        if(currentCanvasMode == CanvasMode::MOVE_POINT) {
            p->setPen(QPen(QColor(0, 0, 0, 255), 1.5));
            mCenter->draw(p);
            mHorizontalRadiusPoint->draw(p);
            mVerticalRadiusPoint->draw(p);

            mFillGradientPoints.drawGradientPoints(p);
            mStrokeGradientPoints.drawGradientPoints(p);
        }
        p->restore();
    }
}


MovablePoint *Circle::getPointAt(QPointF absPtPos, CanvasMode currentCanvasMode)
{
    MovablePoint *pointToReturn = NULL;
    if(currentCanvasMode == MOVE_POINT) {
        pointToReturn = mStrokeGradientPoints.getPointAt(absPtPos);
        if(pointToReturn == NULL) {
            pointToReturn = mFillGradientPoints.getPointAt(absPtPos);
        }
    }
    if(pointToReturn == NULL) {
        if(mCenter->isPointAt(absPtPos)) {
            return mCenter;
        }
        if(mHorizontalRadiusPoint->isPointAt(absPtPos) ) {
            return mHorizontalRadiusPoint;
        }
        if(mVerticalRadiusPoint->isPointAt(absPtPos) ) {
            return mVerticalRadiusPoint;
        }
    }
    return pointToReturn;
}

void Circle::selectAndAddContainedPointsToList(QRectF absRect,
                                                   QList<MovablePoint *> *list)
{
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

void Circle::updatePath()
{
    mPath = QPainterPath();
    QPointF centerPos = mCenter->getRelativePos();
    mPath.addEllipse(centerPos,
                     (centerPos - mHorizontalRadiusPoint->getRelativePos()).x(),
                     (centerPos - mVerticalRadiusPoint->getRelativePos()).y() );

    updateMappedPath();
}

void Circle::centerPivotPosition() {
    mTransformAnimator.setPivot(mCenter->getRelativePos());
}

CircleCenterPoint::CircleCenterPoint(qreal relPosX, qreal relPosY,
                                     BoundingBox *parent,
                                     MovablePointType type) :
    MovablePoint(relPosX, relPosY, parent, type)
{

}



CircleCenterPoint::~CircleCenterPoint()
{

}

void CircleCenterPoint::setVerticalAndHorizontalPoints(
                                                MovablePoint *verticalPoint,
                                                MovablePoint *horizontalPoint)
{
    mVerticalPoint = verticalPoint;
    mHorizontalPoint = horizontalPoint;
}

void CircleCenterPoint::moveBy(QPointF absTranslatione)
{
    mParent->moveBy(absTranslatione);
}

void CircleCenterPoint::moveByAbs(QPointF absTranslatione) {
    mParent->moveBy(absTranslatione);
}

void CircleCenterPoint::startTransform()
{
    mParent->startTransform();
}

void CircleCenterPoint::finishTransform()
{
    mParent->finishTransform();
}

CircleRadiusPoint::CircleRadiusPoint(qreal relPosX, qreal relPosY,
                                     BoundingBox *parent, MovablePointType type,
                                     bool blockX, MovablePoint *centerPoint) :
    MovablePoint(relPosX, relPosY, parent, type)
{
    mCenterPoint = centerPoint;
    mXBlocked = blockX;
}

CircleRadiusPoint::~CircleRadiusPoint()
{

}

void CircleRadiusPoint::moveBy(QPointF absTranslatione)
{
    if(mCenterPoint->isSelected() ) return;
    MovablePoint::moveBy(QPointF( (mXBlocked ? 0. : absTranslatione.x() ),
                         (mXBlocked ? absTranslatione.y() : 0. ) ) );
}

void CircleRadiusPoint::setAbsPosRadius(QPointF pos)
{
    QMatrix combinedM = mParent->getCombinedTransform();
    QPointF newPos = combinedM.inverted().map(pos);
    if(mXBlocked) {
        newPos.setX(getRelativePos().x() );
    } else {
        newPos.setY(getRelativePos().y() );
    }
    setRelativePos(newPos, false );
}

void CircleRadiusPoint::moveByAbs(QPointF absTranslatione) {
    if(mCenterPoint->isSelected() ) return;
    mRelPos.setCurrentValue(mSavedRelPos);
    setAbsPosRadius(getAbsolutePos() + absTranslatione);
}

void CircleRadiusPoint::startTransform()
{
    if(mCenterPoint->isSelected() ) return;
    MovablePoint::startTransform();
}

void CircleRadiusPoint::finishTransform()
{
    if(mCenterPoint->isSelected() ) return;
    MovablePoint::finishTransform();
}
