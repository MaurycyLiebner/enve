#include "circle.h"
#include "canvas.h"
#include "movablepoint.h"
#include "updatescheduler.h"

Circle::Circle(BoxesGroup *parent) : PathBox(parent, TYPE_CIRCLE)
{
    setName("Circle");

    mCenter = new CircleCenterPoint(this, TYPE_PATH_POINT);
    mCenter->setRelativePos(QPointF(0., 0.), false);
    mHorizontalRadiusPoint = new CircleRadiusPoint(this, TYPE_PATH_POINT,
                                                   false, mCenter);
    mCenter->setRelativePos(QPointF(0., 0.), false);
    mVerticalRadiusPoint = new CircleRadiusPoint(this, TYPE_PATH_POINT,
                                                 true, mCenter);
    mCenter->setRelativePos(QPointF(0., 0.), false);

    QrealAnimator *hXAnimator = mHorizontalRadiusPoint->
                                    getRelativePosAnimatorPtr()->getXAnimator();
    addActiveAnimator(hXAnimator);
    hXAnimator->setName("horizontal radius");

    QrealAnimator *vYAnimator = mVerticalRadiusPoint->
                                    getRelativePosAnimatorPtr()->getYAnimator();
    addActiveAnimator(vYAnimator);
    vYAnimator->setName("vertical radius");


    mCenter->setVerticalAndHorizontalPoints(mVerticalRadiusPoint,
                                            mHorizontalRadiusPoint);

    mCenter->setPosAnimatorUpdater(new PathPointUpdater(this) );
    mHorizontalRadiusPoint->setPosAnimatorUpdater(
                new PathPointUpdater(this));
    mVerticalRadiusPoint->setPosAnimatorUpdater(
                new PathPointUpdater(this));

    schedulePathUpdate();
}


#include <QSqlError>
int Circle::saveToSql(int parentId)
{
    int boundingBoxId = PathBox::saveToSql(parentId);

    int horizontalRadiusPointId = mHorizontalRadiusPoint->saveToSql();
    int verticalRadiusPointId = mVerticalRadiusPoint->saveToSql();

    QSqlQuery query;
    if(!query.exec(QString("INSERT INTO circle (boundingboxid, "
                           "horizontalradiuspointid, verticalradiuspointid) "
                "VALUES (%1, %2, %3)").
                arg(boundingBoxId).
                arg(horizontalRadiusPointId).
                arg(verticalRadiusPointId) ) ) {
        qDebug() << query.lastError() << endl << query.lastQuery();
    }

    return boundingBoxId;
}


void Circle::loadFromSql(int boundingBoxId) {
    PathBox::loadFromSql(boundingBoxId);

    QSqlQuery query;
    QString queryStr = "SELECT * FROM circle WHERE boundingboxid = " +
            QString::number(boundingBoxId);
    if(query.exec(queryStr) ) {
        query.next();
        int idHorizontalRadiusPointId = query.record().indexOf("horizontalradiuspointid");
        int idVerticalRadiusPointId = query.record().indexOf("verticalradiuspointid");

        int horizontalRadiusPointId = query.value(idHorizontalRadiusPointId).toInt();
        int verticalRadiusPointId = query.value(idVerticalRadiusPointId).toInt();

        mHorizontalRadiusPoint->loadFromSql(horizontalRadiusPointId);
        mVerticalRadiusPoint->loadFromSql(verticalRadiusPointId);
    } else {
        qDebug() << "Could not load circle with id " << boundingBoxId;
    }

    if(!mPivotChanged) centerPivotPosition();
}

QPointF ellipseQPointFToCanvas(QPointF ellipseCoordPoint, qreal ar, qreal br) {
    return QPointF(ellipseCoordPoint.x() + ar, br - ellipseCoordPoint.y());
}

void Circle::getTopLeftQuadrantVectorPoints(QPointF *leftPtr, QPointF *leftCtrlPtr,
                                            QPointF *topCtrlPtr, QPointF *topPtr) {
    qreal ar = qAbs(mHorizontalRadiusPoint->getRelativePos().x());
    qreal br = qAbs(mVerticalRadiusPoint->getRelativePos().y());

    qreal leftCtrlY = br;
    qreal topCtrlX = -ar;
    qreal xTestLeft = -ar*0.75;
    qreal xTestTop = -ar*0.25;

    qreal maxTopCtrlX = ar/3;
    qreal minLeftCtrlY = br/3;

    qreal testLeftY = br * sqrt(ar*ar - xTestLeft*xTestLeft) / ar;
    qreal testTopY = br * sqrt(ar*ar - xTestTop*xTestTop) / ar;

    QPointF left = QPointF(-ar, 0.);
    QPointF leftCtrl = QPointF(-ar, leftCtrlY);
    QPointF topCtrl = QPointF(topCtrlX, br);
    QPointF top = QPointF(0., br);
    qreal dLeft = 0.;
    qreal dTop = 0.;
    for(int i = 0; i < 100; i++) {
        if(qAbs(dTop) < qAbs(dLeft)*0.5) dTop = -dLeft;
        topCtrlX += dTop;
        if(topCtrlX > maxTopCtrlX) topCtrlX = maxTopCtrlX;

        if(qAbs(dLeft) < qAbs(dTop)*0.5) dLeft = -dTop;
        leftCtrlY += dLeft;
        if(leftCtrlY < minLeftCtrlY) leftCtrlY = minLeftCtrlY;

        leftCtrl.setY(leftCtrlY);
        topCtrl.setX(topCtrlX);

        qreal tLeftTest = tFromX(left.x(),
                         leftCtrl.x(),
                         topCtrl.x(),
                         top.x(), xTestLeft);
        qreal guessTestLeftY = calcCubicBezierVal(left.y(), leftCtrl.y(),
                                  topCtrl.y(), top.y(), tLeftTest);

        qreal tTopTest = tFromX(left.x(),
                         leftCtrl.x(),
                         topCtrl.x(),
                         top.x(), xTestTop);
        qreal guessTestTopY = calcCubicBezierVal(left.y(), leftCtrl.y(),
                                  topCtrl.y(), top.y(), tTopTest);

        dLeft = testLeftY - guessTestLeftY;
        dTop = guessTestTopY - testTopY;
    }

    // from ellipse coordinates to canvas
    *leftPtr = left;
    *leftCtrlPtr = leftCtrl;
    *topCtrlPtr = topCtrl;
    *topPtr = top;
}

VectorPath *Circle::objectToPath()
{
    QPointF left;
    QPointF leftEndCtrl;
    QPointF topStartCtrl;
    QPointF top;

    getTopLeftQuadrantVectorPoints(&left, &leftEndCtrl, &topStartCtrl, &top);

    QPointF right = QPointF(-left.x(), left.y() );
    QPointF bottom = QPointF(top.x(), -top.y() );

    QPointF leftStartCtrl = left + left - leftEndCtrl;
    QPointF topEndCtrl = top + top - topStartCtrl;

    QPointF rightStartCtrl = QPointF(-leftEndCtrl.x(), leftEndCtrl.y() );
    QPointF rightEndCtrl = QPointF(-leftStartCtrl.x(), leftStartCtrl.y() );

    QPointF bottomStartCtrl = QPointF(topEndCtrl.x(), -topEndCtrl.y());
    QPointF bottomEndCtrl = QPointF(topStartCtrl.x(), -topStartCtrl.y());

    VectorPath *newPath = new VectorPath(mParent);
    PathPoint *prevPoint = newPath->addPointRelPos(left, leftStartCtrl, leftEndCtrl);
    PathPoint *leftPoint = prevPoint;
    prevPoint = newPath->addPointRelPos(top, topStartCtrl, topEndCtrl, prevPoint);
    prevPoint = newPath->addPointRelPos(right, rightStartCtrl, rightEndCtrl, prevPoint);
    prevPoint = newPath->addPointRelPos(bottom, bottomStartCtrl, bottomEndCtrl, prevPoint);
    prevPoint->connectToPoint(leftPoint);

    copyTransformationTo(newPath);

    return newPath;
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
    mTransformAnimator.setPivotWithoutChangingTransformation(mCenter->getRelativePos());
}

CircleCenterPoint::CircleCenterPoint(BoundingBox *parent,
                                     MovablePointType type) :
    MovablePoint(parent, type)
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

CircleRadiusPoint::CircleRadiusPoint(BoundingBox *parent, MovablePointType type,
                                     bool blockX, MovablePoint *centerPoint) :
    MovablePoint(parent, type)
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
