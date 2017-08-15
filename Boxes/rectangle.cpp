#include "Boxes/rectangle.h"
#include "canvas.h"
#include "Animators/animatorupdater.h"
#include "gradientpoints.h"

Rectangle::Rectangle() : PathBox(TYPE_RECTANGLE) {
    setName("Rectangle");

    mTopLeftPoint = new RectangleTopLeftPoint(this);
    mTopLeftPoint->setRelativePos(QPointF(0., 0.), false);
    mBottomRightPoint = new RectangleBottomRightPoint(this);
    mBottomRightPoint->setRelativePos(QPointF(0., 0.), false);

    //mTopLeftPoint->setBottomRightPoint(mBottomRightPoint);
    //mBottomRightPoint->setRadiusPoint(mRadiusPoint);

    mTopLeftPoint->prp_setName("top left");
    mBottomRightPoint->prp_setName("bottom right");

    ca_addChildAnimator(mTopLeftPoint);
    ca_addChildAnimator(mBottomRightPoint);

    mXRadiusAnimator.prp_setName("x radius");
    ca_addChildAnimator(&mXRadiusAnimator);
    mXRadiusAnimator.qra_setValueRange(0., 9999.);

    mYRadiusAnimator.prp_setName("y radius");
    ca_addChildAnimator(&mYRadiusAnimator);
    mYRadiusAnimator.qra_setValueRange(0., 9999.);

    prp_setUpdater(new PathPointUpdater(this));
}

Rectangle::~Rectangle()
{

}

#include <QSqlError>
int Rectangle::saveToSql(QSqlQuery *query, const int &parentId) {
    int boundingBoxId = PathBox::saveToSql(query, parentId);

    int bottomRightPointId = mBottomRightPoint->saveToSql(query);
    int topLeftPointId = mTopLeftPoint->saveToSql(query);
    int xRadiusPointId = mXRadiusAnimator.saveToSql(query);
    int yRadiusPointId = mYRadiusAnimator.saveToSql(query);

    if(!query->exec(QString("INSERT INTO rectangle (boundingboxid, "
                           "topleftpointid, bottomrightpointid, "
                           "xradiuspointid, yradiuspointid) "
                "VALUES (%1, %2, %3, %4, %5)").
                arg(boundingBoxId).
                arg(topLeftPointId).
                arg(bottomRightPointId).
                arg(xRadiusPointId).
                arg(yRadiusPointId)) ) {
        qDebug() << query->lastError() << endl << query->lastQuery();
    }

    return boundingBoxId;
}

void Rectangle::duplicateRectanglePointsFrom(
        RectangleTopLeftPoint *topLeftPoint,
        RectangleBottomRightPoint *bottomRightPoint,
        QrealAnimator *xRadiusAnimator,
        QrealAnimator *yRadiusAnimator) {
    topLeftPoint->makeDuplicate(mTopLeftPoint);
    bottomRightPoint->makeDuplicate(mBottomRightPoint);
    xRadiusAnimator->makeDuplicate(&mXRadiusAnimator);
    yRadiusAnimator->makeDuplicate(&mYRadiusAnimator);
}

BoundingBox *Rectangle::createNewDuplicate() {
    return new Rectangle();
}

void Rectangle::loadFromSql(const int &boundingBoxId) {
    PathBox::loadFromSql(boundingBoxId);

    QSqlQuery query;
    QString queryStr = "SELECT * FROM rectangle WHERE boundingboxid = " +
            QString::number(boundingBoxId);
    if(query.exec(queryStr) ) {
        query.next();
        int idBottomRightPointId = query.record().indexOf("bottomrightpointid");
        int idTopLeftPointId = query.record().indexOf("topleftpointid");
        int idXRadiusPointId = query.record().indexOf("xradiuspointid");
        int idYRadiusPointId = query.record().indexOf("yradiuspointid");

        int bottomRightPointId = query.value(idBottomRightPointId).toInt();
        int topLeftPointId = query.value(idTopLeftPointId).toInt();
        int xRadiusPointId = query.value(idXRadiusPointId).toInt();
        int yRadiusPointId = query.value(idYRadiusPointId).toInt();

        mBottomRightPoint->loadFromSql(bottomRightPointId);
        mTopLeftPoint->loadFromSql(topLeftPointId);
        mXRadiusAnimator.loadFromSql(xRadiusPointId);
        mYRadiusAnimator.loadFromSql(yRadiusPointId);
    } else {
        qDebug() << "Could not load rectangle with id " << boundingBoxId;
    }
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
                                getCurrentPointValueAtRelFrame(relFrame));
    SkPoint bottomRight =
            QPointFToSkPoint(mBottomRightPoint->
                                getCurrentPointValueAtRelFrame(relFrame));
    path.addRoundRect(SkRect::MakeLTRB(topLeft.x(), topLeft.y(),
                                       bottomRight.x(), bottomRight.y()),
                      mXRadiusAnimator.getCurrentValueAtRelFrame(relFrame),
                      mYRadiusAnimator.getCurrentValueAtRelFrame(relFrame));
    return path;
}

void Rectangle::setTopLeftPos(const QPointF &pos) {
    mTopLeftPoint->setRelativePos(pos);
}

void Rectangle::setBottomRightPos(const QPointF &pos) {
    mBottomRightPoint->setRelativePos(pos);
}

void Rectangle::setYRadius(const qreal &radiusY) {
    mYRadiusAnimator.qra_setCurrentValue(radiusY);
}

void Rectangle::setXRadius(const qreal &radiusX) {
    mXRadiusAnimator.qra_setCurrentValue(radiusX);
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

RectangleTopLeftPoint::RectangleTopLeftPoint(BoundingBox *parent) :
    MovablePoint(parent, TYPE_PATH_POINT) {

}

RectangleBottomRightPoint::RectangleBottomRightPoint(BoundingBox *parent) :
    MovablePoint(parent, TYPE_PATH_POINT) {

}
