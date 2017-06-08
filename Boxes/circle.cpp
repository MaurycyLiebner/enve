#include "Boxes/circle.h"
#include "canvas.h"
#include "movablepoint.h"
#include "updatescheduler.h"
#include "Animators/animatorupdater.h"
#include "gradientpoints.h"

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
int Circle::saveToSql(QSqlQuery *query, const int &parentId) {
    int boundingBoxId = PathBox::saveToSql(query, parentId);

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
                            const qreal &invScale) {
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

void Circle::updatePath() {
    mPathSk = SkPath();
    mPathSk.addOval(QRectFToSkRect(mRelBoundingRect));

    updateOutlinePathSk();
}

CircleCenterPoint::CircleCenterPoint(BoundingBox *parent,
                                     MovablePointType type) :
    MovablePoint(parent, type) {

}


CircleCenterPoint::~CircleCenterPoint() {

}

void CircleCenterPoint::setVerticalAndHorizontalPoints(
                                                MovablePoint *verticalPoint,
                                                MovablePoint *horizontalPoint) {
    mVerticalPoint = verticalPoint;
    mHorizontalPoint = horizontalPoint;
}

void CircleCenterPoint::moveByRel(const QPointF &absTranslatione) {
    mParent->moveByRel(absTranslatione);
}

void CircleCenterPoint::moveByAbs(const QPointF &absTranslatione) {
    mParent->moveByAbs(absTranslatione);
}

void CircleCenterPoint::startTransform() {
    mParent->startTransform();
}

void CircleCenterPoint::finishTransform() {
    mParent->finishTransform();
}

CircleRadiusPoint::CircleRadiusPoint(BoundingBox *parent,
                                     const MovablePointType &type,
                                     const bool &blockX,
                                     MovablePoint *centerPoint) :
    MovablePoint(parent, type) {
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
    MovablePoint::moveByRel(relTranslationT);
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

void CircleRadiusPoint::setRelativePos(const QPointF &relPos,
                                       const bool &saveUndoRedo) {
    if(mXBlocked) {
        mYAnimator->qra_setCurrentValue(relPos.y(), saveUndoRedo);
    } else {
        mXAnimator->qra_setCurrentValue(relPos.x(), saveUndoRedo);
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
