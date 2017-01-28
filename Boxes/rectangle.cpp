#include "Boxes/rectangle.h"
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
int Rectangle::saveToSql(QSqlQuery *query, int parentId)
{
    int boundingBoxId = PathBox::saveToSql(query, parentId);

    int bottomRightPointId = mTopLeftPoint->saveToSql(query);
    int topLeftPointId = mTopLeftPoint->saveToSql(query);
    int radiusPointId = mRadiusPoint->saveToSql(query);

    if(!query->exec(QString("INSERT INTO rectangle (boundingboxid, "
                           "topleftpointid, bottomrightpointid, "
                           "radiuspointid) "
                "VALUES (%1, %2, %3, %4)").
                arg(boundingBoxId).
                arg(topLeftPointId).
                arg(bottomRightPointId).
                arg(radiusPointId) ) ) {
        qDebug() << query->lastError() << endl << query->lastQuery();
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
        if(mTopLeftPoint->isPointAtAbsPos(absPtPos)) {
            return mTopLeftPoint;
        }
        if(mBottomRightPoint->isPointAtAbsPos(absPtPos) ) {
            return mBottomRightPoint;
        }
        if(mRadiusPoint->isPointAtAbsPos(absPtPos) ) {
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

    updateOutlinePath();
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
