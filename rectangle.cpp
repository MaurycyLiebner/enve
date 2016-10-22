#include "rectangle.h"
#include "canvas.h"

Rectangle::Rectangle(BoxesGroup *parent) : PathBox(parent, TYPE_RECTANGLE)
{
    setName("Rectangle");

    mTopLeftPoint = new RectangleTopLeftPoint(this);
    mTopLeftPoint->setRelativePos(QPointF(0., 0.), false);
    mBottomRightPoint = new RectangleBottomRightPoint(this);
    mBottomRightPoint->setRelativePos(QPointF(0., 0.), false);
    mRadiusPoint = new RectangleRadiusPoint(this);
    mRadiusPoint->setRelativePos(QPointF(0., 0.), false);

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

    mRadiusAnimator = mRadiusPoint->
            getRelativePosAnimatorPtr()->getYAnimator();

    mRadiusAnimator->setName("radius");
    addActiveAnimator(mRadiusAnimator);

    mBottomRightPoint->setPosAnimatorUpdater(new RectangleBottomRightUpdater(this) );
    mRadiusPoint->setPosAnimatorUpdater(new PathPointUpdater(this) );
}

Rectangle::~Rectangle()
{

}

#include <QSqlError>
int Rectangle::saveToSql(int parentId)
{
    int boundingBoxId = PathBox::saveToSql(parentId);

    int bottomRightPointId = mTopLeftPoint->saveToSql();
    int topLeftPointId = mTopLeftPoint->saveToSql();
    int radiusPointId = mRadiusPoint->saveToSql();

    QSqlQuery query;
    if(!query.exec(QString("INSERT INTO rectangle (boundingboxid, "
                           "topleftpointid, bottomrightpointid, "
                           "radiuspointid) "
                "VALUES (%1, %2, %3, %4)").
                arg(boundingBoxId).
                arg(topLeftPointId).
                arg(bottomRightPointId).
                arg(radiusPointId) ) ) {
        qDebug() << query.lastError() << endl << query.lastQuery();
    }

    return boundingBoxId;
}


void Rectangle::loadFromSql(int boundingBoxId) {
    PathBox::loadFromSql(boundingBoxId);

    QSqlQuery query;
    QString queryStr = "SELECT * FROM rectangle WHERE boundingboxid = " +
            QString::number(boundingBoxId);
    if(query.exec(queryStr) ) {
        query.next();
        int idBottomRightPointId = query.record().indexOf("bottomrightpointid");
        int idTopLeftPointId = query.record().indexOf("topleftpointid");
        int idRadiusPointId = query.record().indexOf("radiuspointid");

        int bottomRightPointId = query.value(idBottomRightPointId).toInt();
        int topLeftPointId = query.value(idTopLeftPointId).toInt();
        int radiusPointId = query.value(idRadiusPointId).toInt();

        mBottomRightPoint->loadFromSql(bottomRightPointId);
        mTopLeftPoint->loadFromSql(topLeftPointId);
        mRadiusPoint->loadFromSql(radiusPointId);
    } else {
        qDebug() << "Could not load rectangle with id " << boundingBoxId;
    }

    if(!mPivotChanged) centerPivotPosition();
}

VectorPath *Rectangle::objectToPath()
{
    qreal radius = mRadiusAnimator->getCurrentValue();
    QPointF topLeft = QPointF(0., 0.);
    QPointF bottomRight = mBottomRightPoint->getRelativePos();
    QPointF topRight = QPointF(bottomRight.x(), topLeft.y());
    QPointF bottomLeft = QPointF(topLeft.x(), bottomRight.y());

    radius = qclamp(radius, 0., bottomRight.y()*0.5);

    VectorPath *newPath = new VectorPath(mParent);
    if(isZero(radius) ) {
        PathPoint *prevPoint = newPath->addPointRelPos(topLeft);
        PathPoint *topLeftPoint = prevPoint;
        prevPoint = newPath->addPointRelPos(topRight, prevPoint);
        prevPoint = newPath->addPointRelPos(bottomRight, prevPoint);
        prevPoint = newPath->addPointRelPos(bottomLeft, prevPoint);
        prevPoint->connectToPoint(topLeftPoint);
    } else {
        qreal kappa = 0.552228474;
        qreal oneMinusKappa = 1. - kappa;
        qreal k = radius*oneMinusKappa;

        QPointF topLeft1 = topLeft + QPointF(0., radius);
        QPointF topLeft1End = topLeft + QPointF(0., k);
        //
        QPointF topLeft2 = topLeft + QPointF(radius, 0.);
        QPointF topLeft2Start = topLeft + QPointF(k, 0.);
        //
        QPointF topRight1 = topRight - QPointF(radius, 0.);
        QPointF topRight1End = topRight - QPointF(k, 0.);
        //
        QPointF topRight2 = topRight + QPointF(0., radius);
        QPointF topRight2Start = topRight + QPointF(0., k);
        //
        QPointF bottomRight1 = bottomRight - QPointF(0., radius);
        QPointF bottomRight1End = bottomRight - QPointF(0., k);
        //
        QPointF bottomRight2 = bottomRight - QPointF(radius, 0.);
        QPointF bottomRight2Start = bottomRight - QPointF(k, 0.);
        //
        QPointF bottomLeft1 = bottomLeft + QPointF(radius, 0.);
        QPointF bottomLeft1End = bottomLeft + QPointF(k, 0.);
        //
        QPointF bottomLeft2 = bottomLeft - QPointF(0., radius);
        QPointF bottomLeft2Start = bottomLeft - QPointF(0., k);

        PathPoint *prevPoint = newPath->addPointRelPos(topLeft1,
                                                       topLeft1, topLeft1End);
        prevPoint->setStartCtrlPtEnabled(false);
        PathPoint *topLeft1Point = prevPoint;
        prevPoint = newPath->addPointRelPos(topLeft2,
                                            topLeft2Start, topLeft2,
                                            prevPoint);
        prevPoint->setEndCtrlPtEnabled(false);
        prevPoint = newPath->addPointRelPos(topRight1,
                                            topRight1, topRight1End,
                                            prevPoint);
        prevPoint->setStartCtrlPtEnabled(false);
        prevPoint = newPath->addPointRelPos(topRight2,
                                            topRight2Start, topRight2,
                                            prevPoint);
        prevPoint->setEndCtrlPtEnabled(false);
        prevPoint = newPath->addPointRelPos(bottomRight1,
                                            bottomRight1, bottomRight1End,
                                            prevPoint);
        prevPoint->setStartCtrlPtEnabled(false);
        prevPoint = newPath->addPointRelPos(bottomRight2,
                                            bottomRight2Start, bottomRight2,
                                            prevPoint);
        prevPoint->setEndCtrlPtEnabled(false);
        prevPoint = newPath->addPointRelPos(bottomLeft1,
                                            bottomLeft1, bottomLeft1End,
                                            prevPoint);
        prevPoint->setStartCtrlPtEnabled(false);
        prevPoint = newPath->addPointRelPos(bottomLeft2,
                                            bottomLeft2Start, bottomLeft2,
                                            prevPoint);
        prevPoint->setEndCtrlPtEnabled(false);
        prevPoint->connectToPoint(topLeft1Point);
    }

    copyTransformationTo(newPath);

    return newPath;
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
            setValueRange(0., bttmPos.y()*0.5 );
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
    mTransformAnimator.setPivotWithoutChangingTransformation(mBottomRightPoint->getRelativePos() * 0.5);
}

RectangleTopLeftPoint::RectangleTopLeftPoint(BoundingBox *parent) :
    MovablePoint(parent, TYPE_PATH_POINT) {

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

RectangleBottomRightPoint::RectangleBottomRightPoint(BoundingBox *parent) :
    MovablePoint(parent, TYPE_PATH_POINT) {

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

RectangleRadiusPoint::RectangleRadiusPoint(BoundingBox *parent) :
    MovablePoint(parent, TYPE_PATH_POINT) {

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
