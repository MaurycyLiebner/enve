#include "movablepoint.h"
#include "vectorpath.h"
#include "mainwindow.h"
#include "undoredo.h"

MovablePoint::MovablePoint(BoundingBox *parent,
                           MovablePointType type,
                           qreal radius) : Transformable()
{
    mType = type;
    mRadius = radius;
    mParent = parent;

    mRelPos.blockPointer();
}

void MovablePoint::loadFromSql(int movablePointId) {
    QSqlQuery query;

    QString queryStr = "SELECT * FROM movablepoint WHERE id = " +
            QString::number(movablePointId);
    if(query.exec(queryStr)) {
        query.next();
        int idposanimator = query.record().indexOf("posanimatorid");

        mRelPos.loadFromSql(query.value(idposanimator).toInt() );
    } else {
        qDebug() << "Could not load movablepoint with id " << movablePointId;
    }
}

void MovablePoint::startTransform()
{
    mRelPos.startTransform();
    mTransformStarted = true;
    mSavedRelPos = getRelativePos();
}

void MovablePoint::finishTransform()
{
    if(mTransformStarted) {
        mTransformStarted = false;
        mRelPos.finishTransform();
    }
}

void MovablePoint::setAbsolutePos(QPointF pos, bool saveUndoRedo)
{
    QMatrix combinedM = mParent->getCombinedTransform();
    QPointF newPos = combinedM.inverted().map(pos);
    setRelativePos(newPos, saveUndoRedo );
}

void MovablePoint::setRelativePos(QPointF relPos, bool saveUndoRedo)
{
    mRelPos.setCurrentValue(relPos, saveUndoRedo);
}

QPointF MovablePoint::getRelativePos()
{
    return mRelPos.getCurrentValue();
}

QPointF MovablePoint::mapRelativeToAbsolute(QPointF relPos) {
    return mParent->getCombinedTransform().map(relPos);
}

QPointF MovablePoint::getAbsolutePos()
{
    return mapRelativeToAbsolute(getRelativePos());
}

void MovablePoint::draw(QPainter *p)
{
    if(isHidden()) {
        return;
    }
    if(mSelected) {
        p->setBrush(QColor(255, 0, 0));
    } else {
        p->setBrush(QColor(255, 175, 175));
    }
    QPointF absPos = getAbsolutePos();
    p->drawEllipse(absPos,
                   mRadius, mRadius);
    if(mRelPos.isKeyOnCurrentFrame()) {
        p->save();
        p->setBrush(Qt::red);
        p->setPen(QPen(Qt::black, 1.) );
        p->drawEllipse(absPos, 4, 4);
        p->restore();
    }
}

BoundingBox *MovablePoint::getParent()
{
    return mParent;
}

bool MovablePoint::isPointAt(QPointF absPoint)
{
    if(isHidden()) {
        return false;
    }
    QPointF dist = getAbsolutePos() - absPoint;
    return (dist.x()*dist.x() + dist.y()*dist.y() < mRadius*mRadius);
}

bool MovablePoint::isContainedInRect(QRectF absRect)
{
    if(isHidden() || (isCtrlPoint() && !BoxesGroup::getCtrlsAlwaysVisible()) ) {
        return false;
    }
    return absRect.contains(getAbsolutePos());
}

void MovablePoint::moveBy(QPointF relTranslation) {
    mRelPos.incSavedValueToCurrentValue(relTranslation.x(), relTranslation.y());
}

void MovablePoint::moveByAbs(QPointF absTranslatione) {
    moveToAbs(mapRelativeToAbsolute(mRelPos.getSavedValue()) + absTranslatione);
}

void MovablePoint::moveToAbs(QPointF absPos)
{
    setAbsolutePos(absPos, false);
}

void MovablePoint::scale(qreal scaleBy) {
    scale(scaleBy, scaleBy);
}

void MovablePoint::cancelTransform()
{
    mRelPos.cancelTransform();
    //setRelativePos(mSavedRelPos, false);
}

void MovablePoint::setRadius(qreal radius)
{
    mRadius = radius;
}

bool MovablePoint::isBeingTransformed()
{
    return mSelected || mParent->isSelected();
}

#include <QSqlError>
int MovablePoint::saveToSql()
{
    int posAnimatorId = mRelPos.saveToSql();
    QSqlQuery query;
    if(!query.exec(QString("INSERT INTO movablepoint (posanimatorid) "
                "VALUES (%1)").
                arg(posAnimatorId) ) ) {
        qDebug() << query.lastError() << endl << query.lastQuery();
    }
    return query.lastInsertId().toInt();
}

void MovablePoint::setPosAnimatorUpdater(AnimatorUpdater *updater)
{
    mRelPos.setUpdater(updater);
}

QPointFAnimator *MovablePoint::getRelativePosAnimatorPtr()
{
    return &mRelPos;
}

qreal MovablePoint::getRadius()
{
    return mRadius;
}

void MovablePoint::updateAfterFrameChanged(int frame)
{
    mRelPos.setFrame(frame);
}

void MovablePoint::rotateBy(qreal rot)
{
    QMatrix rotMatrix;
    rotMatrix.translate(-mSavedTransformPivot.x(), -mSavedTransformPivot.y());
    rotMatrix.rotate(rot);
    rotMatrix.translate(mSavedTransformPivot.x(), mSavedTransformPivot.y());
    setRelativePos(mSavedRelPos*rotMatrix, false);
}

void MovablePoint::scale(qreal scaleXBy, qreal scaleYBy)
{
    QMatrix scaleMatrix;
    scaleMatrix.translate(-mSavedTransformPivot.x(), -mSavedTransformPivot.y());
    scaleMatrix.scale(scaleXBy, scaleYBy);
    scaleMatrix.translate(mSavedTransformPivot.x(), mSavedTransformPivot.y());
    setRelativePos(mSavedRelPos*scaleMatrix, false);
}

void MovablePoint::saveTransformPivot(QPointF absPivot)
{
    mSavedTransformPivot = -mParent->mapAbsPosToRel(absPivot);
}

void MovablePoint::select()
{
    mSelected = true;
}

void MovablePoint::deselect()
{
    mSelected = false;
}

void MovablePoint::remove()
{

}

void MovablePoint::hide()
{
    mHidden = true;
    deselect();
}

void MovablePoint::show()
{
    mHidden = false;
}

bool MovablePoint::isHidden()
{
    return mHidden;
}

bool MovablePoint::isVisible()
{
    return !isHidden();
}

void MovablePoint::setVisible(bool bT)
{
    if(bT) {
        show();
    } else {
        hide();
    }
}

bool MovablePoint::isPathPoint()
{
    return mType == MovablePointType::TYPE_PATH_POINT;
}

bool MovablePoint::isPivotPoint()
{
    return mType == MovablePointType::TYPE_PIVOT_POINT;
}

bool MovablePoint::isCtrlPoint()
{
    return mType == MovablePointType::TYPE_CTRL_POINT;
}
