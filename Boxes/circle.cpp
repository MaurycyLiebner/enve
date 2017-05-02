#include "Boxes/circle.h"
#include "canvas.h"
#include "movablepoint.h"
#include "updatescheduler.h"

Circle::Circle(BoxesGroup *parent) :
    PathBox(parent, TYPE_CIRCLE) {
    setName("Circle");

    mCenter = new CircleCenterPoint(this, TYPE_PATH_POINT);
    mCenter->setRelativePos(QPointF(0., 0.), false);
    mHorizontalRadiusPoint = new CircleRadiusPoint(this, TYPE_PATH_POINT,
                                                   false, mCenter);
    mHorizontalRadiusPoint->setRelativePos(QPointF(10., 0.), false);
    mVerticalRadiusPoint = new CircleRadiusPoint(this, TYPE_PATH_POINT,
                                                 true, mCenter);
    mVerticalRadiusPoint->setRelativePos(QPointF(0., 10.), false);

    QrealAnimator *hXAnimator = mHorizontalRadiusPoint->getXAnimator();
    ca_addChildAnimator(hXAnimator);
    hXAnimator->prp_setName("horizontal radius");

    QrealAnimator *vYAnimator = mVerticalRadiusPoint->getYAnimator();
    ca_addChildAnimator(vYAnimator);
    vYAnimator->prp_setName("vertical radius");


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
int Circle::prp_saveToSql(QSqlQuery *query, const int &parentId)
{
    int boundingBoxId = PathBox::prp_saveToSql(query, parentId);

    int horizontalRadiusPointId = mHorizontalRadiusPoint->prp_saveToSql(query);
    int verticalRadiusPointId = mVerticalRadiusPoint->prp_saveToSql(query);

    if(!query->exec(QString("INSERT INTO circle (boundingboxid, "
                           "horizontalradiuspointid, verticalradiuspointid) "
                "VALUES (%1, %2, %3)").
                arg(boundingBoxId).
                arg(horizontalRadiusPointId).
                arg(verticalRadiusPointId) ) ) {
        qDebug() << query->lastError() << endl << query->lastQuery();
    }

    return boundingBoxId;
}


void Circle::prp_loadFromSql(const int &boundingBoxId) {
    PathBox::prp_loadFromSql(boundingBoxId);

    QSqlQuery query;
    QString queryStr = "SELECT * FROM circle WHERE boundingboxid = " +
            QString::number(boundingBoxId);
    if(query.exec(queryStr) ) {
        query.next();
        int idHorizontalRadiusPointId = query.record().indexOf("horizontalradiuspointid");
        int idVerticalRadiusPointId = query.record().indexOf("verticalradiuspointid");

        int horizontalRadiusPointId = query.value(idHorizontalRadiusPointId).toInt();
        int verticalRadiusPointId = query.value(idVerticalRadiusPointId).toInt();

        mHorizontalRadiusPoint->prp_loadFromSql(horizontalRadiusPointId);
        mVerticalRadiusPoint->prp_loadFromSql(verticalRadiusPointId);
    } else {
        qDebug() << "Could not load circle with id " << boundingBoxId;
    }

    if(!mPivotChanged) centerPivotPosition();
}

void Circle::duplicateCirclePointsFrom(
        CircleCenterPoint *center,
        CircleRadiusPoint *horizontalRadiusPoint,
        CircleRadiusPoint *verticalRadiusPoint) {
    center->makeDuplicate(mCenter);
    horizontalRadiusPoint->makeDuplicate(mHorizontalRadiusPoint);
    verticalRadiusPoint->makeDuplicate(mVerticalRadiusPoint);
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

void Circle::setRadius(qreal radius) {
    setHorizontalRadius(radius);
    setVerticalRadius(radius);
}

void Circle::drawSelected(QPainter *p, const CanvasMode &currentCanvasMode) {
    if(isVisibleAndInVisibleDurationRect()) {
        p->save();
        drawBoundingRect(p);
        if(currentCanvasMode == CanvasMode::MOVE_POINT) {
            p->setPen(QPen(QColor(0, 0, 0, 255), 1.5));
            mCenter->draw(p);
            mHorizontalRadiusPoint->draw(p);
            mVerticalRadiusPoint->draw(p);

            mFillGradientPoints->drawGradientPoints(p);
            mStrokeGradientPoints->drawGradientPoints(p);
        } else if(currentCanvasMode == MOVE_PATH) {
            mTransformAnimator->getPivotMovablePoint()->draw(p);
        }
        p->restore();
    }
}


MovablePoint *Circle::getPointAtAbsPos(const QPointF &absPtPos,
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

    updateOutlinePath();
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

void CircleCenterPoint::moveByRel(QPointF absTranslatione)
{
    mParent->moveByRel(absTranslatione);
}

void CircleCenterPoint::moveByAbs(QPointF absTranslatione) {
    mParent->moveByAbs(absTranslatione);
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

void CircleRadiusPoint::moveByRel(QPointF relTranslation)
{
    if(mCenterPoint->isSelected() ) return;
    if(mXBlocked) {
        relTranslation.setX(0.);
    } else {
        relTranslation.setY(0.);
    }
    MovablePoint::moveByRel(relTranslation);
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

void CircleRadiusPoint::moveByAbs(QPointF absTranslatione) {
    if(mCenterPoint->isSelected() ) return;
    MovablePoint::moveByAbs(absTranslatione);
    //setCurrentValue(mSavedRelPos);
    //setAbsPosRadius(getAbsolutePos() + absTranslatione);
}

void CircleRadiusPoint::setRelativePos(const QPointF &relPos, const bool &saveUndoRedo) {
    if(mXBlocked) {
        mYAnimator->qra_setCurrentValue(relPos.y(), saveUndoRedo);
    } else {
        mXAnimator->qra_setCurrentValue(relPos.x(), saveUndoRedo);
    }
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
