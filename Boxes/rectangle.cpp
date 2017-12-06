#include "Boxes/rectangle.h"
#include "canvas.h"
#include "Animators/animatorupdater.h"
#include "gradientpoints.h"

Rectangle::Rectangle() : PathBox(TYPE_RECTANGLE) {
    setName("Rectangle");

    mTopLeftPoint = new RectangleTopLeftPoint(mTransformAnimator.data());
    mTopLeftPoint->setRelativePos(QPointF(0., 0.));
    mBottomRightPoint = new RectangleBottomRightPoint(mTransformAnimator.data());
    mBottomRightPoint->setRelativePos(QPointF(0., 0.));

    //mTopLeftPoint->setBottomRightPoint(mBottomRightPoint);
    //mBottomRightPoint->setRadiusPoint(mRadiusPoint);

    mTopLeftPoint->prp_setName("top left");
    mBottomRightPoint->prp_setName("bottom right");

    ca_addChildAnimator(mTopLeftPoint);
    ca_addChildAnimator(mBottomRightPoint);

    mRadiusPoint = new QPointFAnimator();
    mRadiusPoint->prp_setName("round radius");
    ca_addChildAnimator(mRadiusPoint);
    mRadiusPoint->setValuesRange(0., 9999.);

    prp_setUpdater(new NodePointUpdater(this));
}

Rectangle::~Rectangle() {

}

void Rectangle::startAllPointsTransform() {
    mTopLeftPoint->startTransform();
    mBottomRightPoint->startTransform();
    startTransform();
}

void Rectangle::finishAllPointsTransform() {
    mTopLeftPoint->finishTransform();
    mBottomRightPoint->finishTransform();
    finishTransform();
}

SkPath Rectangle::getPathAtRelFrame(const int &relFrame) {
    SkPath path;
    SkPoint topLeft =
            QPointFToSkPoint(mTopLeftPoint->
                                getCurrentEffectivePointValueAtRelFrame(relFrame));
    SkPoint bottomRight =
            QPointFToSkPoint(mBottomRightPoint->
                                getCurrentEffectivePointValueAtRelFrame(relFrame));
    QPointF radiusAtFrame =
            mRadiusPoint->getCurrentEffectivePointValueAtRelFrame(relFrame);
    path.addRoundRect(SkRect::MakeLTRB(topLeft.x(), topLeft.y(),
                                       bottomRight.x(), bottomRight.y()),
                      radiusAtFrame.x(), radiusAtFrame.y());
    return path;
}

void Rectangle::setTopLeftPos(const QPointF &pos) {
    mTopLeftPoint->setRelativePos(pos);
}

void Rectangle::setBottomRightPos(const QPointF &pos) {
    mBottomRightPoint->setRelativePos(pos);
}

void Rectangle::setYRadius(const qreal &radiusY) {
    mRadiusPoint->getYAnimator()->qra_setCurrentValue(radiusY);
}

void Rectangle::setXRadius(const qreal &radiusX) {
    mRadiusPoint->getXAnimator()->qra_setCurrentValue(radiusX);
}

void Rectangle::moveSizePointByAbs(const QPointF &absTrans) {
    mBottomRightPoint->moveByAbs(absTrans);
}

MovablePoint *Rectangle::getBottomRightPoint() {
    return mBottomRightPoint;
}

void Rectangle::drawSelectedSk(SkCanvas *canvas,
                               const CanvasMode &currentCanvasMode,
                               const SkScalar &invScale) {
    if(isVisibleAndInVisibleDurationRect()) {
        canvas->save();
        drawBoundingRectSk(canvas, invScale);
        if(currentCanvasMode == CanvasMode::MOVE_POINT) {
            mTopLeftPoint->drawSk(canvas, invScale);
            mBottomRightPoint->drawSk(canvas, invScale);

            mFillGradientPoints->drawGradientPointsSk(canvas, invScale);
            mStrokeGradientPoints->drawGradientPointsSk(canvas, invScale);
        } else if(currentCanvasMode == MOVE_PATH) {
            mTransformAnimator->getPivotMovablePoint()->
                    drawSk(canvas, invScale);
        }
        canvas->restore();
    }
}


MovablePoint *Rectangle::getPointAtAbsPos(const QPointF &absPtPos,
                                    const CanvasMode &currentCanvasMode,
                                    const qreal &canvasScaleInv) {
    MovablePoint *pointToReturn = PathBox::getPointAtAbsPos(absPtPos,
                                                            currentCanvasMode,
                                                            canvasScaleInv);
    if(pointToReturn == NULL) {
        if(mTopLeftPoint->isPointAtAbsPos(absPtPos, canvasScaleInv)) {
            return mTopLeftPoint;
        }
        if(mBottomRightPoint->isPointAtAbsPos(absPtPos, canvasScaleInv) ) {
            return mBottomRightPoint;
        }
    }
    return pointToReturn;
}

void Rectangle::selectAndAddContainedPointsToList(const QRectF &absRect,
                                                  QList<MovablePoint *> *list) {
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
}

RectangleTopLeftPoint::RectangleTopLeftPoint(BasicTransformAnimator *parent) :
    PointAnimator(parent, TYPE_PATH_POINT) {

}

RectangleBottomRightPoint::RectangleBottomRightPoint(BasicTransformAnimator *parent) :
    PointAnimator(parent, TYPE_PATH_POINT) {

}
