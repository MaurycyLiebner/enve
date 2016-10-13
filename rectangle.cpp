#include "rectangle.h"
#include "canvas.h"

Rectangle::Rectangle(BoxesGroup *parent) : PathBox(parent, TYPE_RECTANGLE)
{
    setName("Rectangle");

    mTopLeftPoint = new RectangleTopLeftPoint(0., 0.,
                                              this);
    mBottomRightPoint = new RectangleBottomRightPoint(0., 0.,
                                                      this);
    mRadiusPoint = new RectangleRadiusPoint(0., 0.,
                                            this);

    mTopLeftPoint->setBottomRightPoint(mBottomRightPoint);
    mRadiusPoint->setPoints(mTopLeftPoint, mBottomRightPoint);
    mBottomRightPoint->setPoints(mTopLeftPoint, mRadiusPoint);

    QrealAnimator *widthAnimator = mBottomRightPoint->
            getRelativePosAnimatorPtr()->getXAnimator();
    QrealAnimator *heightAnimator = mBottomRightPoint->
            getRelativePosAnimatorPtr()->getYAnimator();

    widthAnimator->setName("width");
    heightAnimator->setName("height");

    addActiveAnimator(widthAnimator);
    addActiveAnimator(heightAnimator);

    QrealAnimator *radiusAnimator = mRadiusPoint->
            getRelativePosAnimatorPtr()->getYAnimator();

    radiusAnimator->setName("radius");
    addActiveAnimator(radiusAnimator);

    mBottomRightPoint->setPosAnimatorUpdater(new RectangleBottomRightUpdater(this) );
    mRadiusPoint->setPosAnimatorUpdater(new PathPointUpdater(this) );
}

Rectangle::~Rectangle()
{

}

void Rectangle::updateAfterFrameChanged(int currentFrame)
{
    mBottomRightPoint->updateAfterFrameChanged(currentFrame);
    mRadiusPoint->updateAfterFrameChanged(currentFrame);
    PathBox::updateAfterFrameChanged(currentFrame);
}

void Rectangle::updateRadiusXAndRange() {
    QPointF bttmPos = mBottomRightPoint->getRelativePos();
    mRadiusPoint->getRelativePosAnimatorPtr()->getXAnimator()->
            setValueRange(bttmPos.x(), bttmPos.x());

    mRadiusPoint->getRelativePosAnimatorPtr()->getYAnimator()->
            setValueRange(0., bttmPos.y() - mRadiusPoint->getRadius() );
}

void Rectangle::startAllPointsTransform() {
    mBottomRightPoint->startTransform();
    mRadiusPoint->startTransform();
    startTransform();
}

void Rectangle::moveSizePointByAbs(QPointF absTrans) {
    mBottomRightPoint->moveByAbs(absTrans);
}

void Rectangle::drawSelected(QPainter *p, CanvasMode currentCanvasMode)
{
    if(mVisible) {
        p->save();
        drawBoundingRect(p);
        if(currentCanvasMode == CanvasMode::MOVE_POINT) {
            p->setPen(QPen(QColor(0, 0, 0, 255), 1.5));
            mTopLeftPoint->draw(p);
            mBottomRightPoint->draw(p);
            mRadiusPoint->draw(p);

            mFillGradientPoints.drawGradientPoints(p);
            mStrokeGradientPoints.drawGradientPoints(p);
        }
        p->restore();
    }
}


MovablePoint *Rectangle::getPointAt(QPointF absPtPos, CanvasMode currentCanvasMode)
{
    MovablePoint *pointToReturn = NULL;
    if(currentCanvasMode == MOVE_POINT) {
        pointToReturn = mStrokeGradientPoints.getPointAt(absPtPos);
        if(pointToReturn == NULL) {
            pointToReturn = mFillGradientPoints.getPointAt(absPtPos);
        }
    }
    if(pointToReturn == NULL) {
        if(mTopLeftPoint->isPointAt(absPtPos)) {
            return mTopLeftPoint;
        }
        if(mBottomRightPoint->isPointAt(absPtPos) ) {
            return mBottomRightPoint;
        }
        if(mRadiusPoint->isPointAt(absPtPos) ) {
            return mRadiusPoint;
        }
    }
    return pointToReturn;
}

void Rectangle::selectAndAddContainedPointsToList(QRectF absRect,
                                                   QList<MovablePoint *> *list)
{
    if(!mTopLeftPoint->isSelected()) {
        if(mTopLeftPoint->isContainedInRect(absRect)) {
            mTopLeftPoint->select();
            list->append(mTopLeftPoint);
        }
    }
    if(!mBottomRightPoint->isSelected()) {
        if(mBottomRightPoint->isContainedInRect(absRect)) {
            mBottomRightPoint->select();
            list->append(mBottomRightPoint);
        }
    }
    if(!mRadiusPoint->isSelected()) {
        if(mRadiusPoint->isContainedInRect(absRect)) {
            mRadiusPoint->select();
            list->append(mRadiusPoint);
        }
    }
}

void Rectangle::updatePath()
{
    mPath = QPainterPath();
    QPointF botPos = mBottomRightPoint->getRelativePos();
    QPointF radPos = mRadiusPoint->getRelativePos();
    qreal radius = radPos.y();
    mPath.addRoundedRect(0., 0., botPos.x(), botPos.y(), radius, radius);

    updateMappedPath();
}

void Rectangle::centerPivotPosition() {
    mTransformAnimator.setPivot(mBottomRightPoint->getRelativePos() * 0.5);
}

RectangleTopLeftPoint::RectangleTopLeftPoint(qreal relPosX, qreal relPosY,
                                             BoundingBox *parent) :
    MovablePoint(relPosX, relPosY, parent, TYPE_PATH_POINT) {

}

void RectangleTopLeftPoint::moveBy(QPointF absTranslatione) {
    mParent->moveBy(absTranslatione);
    if(mBottomRightPoint->isSelected()) return;
    mBottomRightPoint->MovablePoint::moveBy(-absTranslatione);
}

void RectangleTopLeftPoint::moveByAbs(QPointF absTranslatione) {
    mParent->moveBy(absTranslatione);
    if(mBottomRightPoint->isSelected()) return;
    mBottomRightPoint->MovablePoint::moveByAbs(-absTranslatione);
}

void RectangleTopLeftPoint::startTransform() {
    mParent->startTransform();
    if(mBottomRightPoint->isSelected()) return;
    mBottomRightPoint->MovablePoint::startTransform();
}

void RectangleTopLeftPoint::finishTransform() {
    mParent->finishTransform();
    if(mBottomRightPoint->isSelected()) return;
    mBottomRightPoint->MovablePoint::finishTransform();
}

void RectangleTopLeftPoint::setBottomRightPoint(
        MovablePoint *bottomRightPoint) {
    mBottomRightPoint = bottomRightPoint;
}

RectangleBottomRightPoint::RectangleBottomRightPoint(qreal relPosX, qreal relPosY,
                                                     BoundingBox *parent) :
    MovablePoint(relPosX, relPosY, parent, TYPE_PATH_POINT) {

}

void RectangleBottomRightPoint::setPoints(MovablePoint *topLeftPoint,
                                          MovablePoint *radiusPoint) {
    mTopLeftPoint = topLeftPoint;
    mRadiusPoint = radiusPoint;
}

void RectangleBottomRightPoint::moveBy(QPointF absTranslatione) {
    if(mTopLeftPoint->isSelected()) return;
    MovablePoint::moveBy(absTranslatione);
}

void RectangleBottomRightPoint::moveByAbs(QPointF absTranslatione) {
    if(mTopLeftPoint->isSelected()) return;
    MovablePoint::moveByAbs(absTranslatione);
}

void RectangleBottomRightPoint::startTransform() {
    if(mTopLeftPoint->isSelected()) return;
    MovablePoint::startTransform();
}

void RectangleBottomRightPoint::finishTransform() {
    if(mTopLeftPoint->isSelected()) return;
    MovablePoint::finishTransform();

}

RectangleRadiusPoint::RectangleRadiusPoint(qreal relPosX, qreal relPosY,
                                           BoundingBox *parent) :
    MovablePoint(relPosX, relPosY, parent, TYPE_PATH_POINT) {

}

void RectangleRadiusPoint::setPoints(MovablePoint *topLeftPoint,
                                     MovablePoint *bottomRightPoint) {
    mTopLeftPoint = topLeftPoint;
    mBottomRightPoint = bottomRightPoint;
}

void RectangleRadiusPoint::moveBy(QPointF absTranslatione) {
    if(mTopLeftPoint->isSelected() || mBottomRightPoint->isSelected() ) return;
    MovablePoint::moveBy(QPointF( 0., absTranslatione.y()) );
}

void RectangleRadiusPoint::setAbsPosRadius(QPointF pos) {
    QMatrix combinedM = mParent->getCombinedTransform();
    QPointF newPos = combinedM.inverted().map(pos);

    setRelativePos(newPos, false );
}

void RectangleRadiusPoint::moveByAbs(QPointF absTranslatione) {
    if(mTopLeftPoint->isSelected() || mBottomRightPoint->isSelected() ) return;
    mRelPos.setCurrentValue(mSavedRelPos);
    setAbsPosRadius(getAbsolutePos() + absTranslatione);
}

void RectangleRadiusPoint::startTransform() {
    if(mTopLeftPoint->isSelected() || mBottomRightPoint->isSelected() ) return;
    MovablePoint::startTransform();
}

void RectangleRadiusPoint::finishTransform() {
    if(mTopLeftPoint->isSelected() || mBottomRightPoint->isSelected() ) return;
    MovablePoint::finishTransform();
}
