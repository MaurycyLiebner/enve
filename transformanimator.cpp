#include "transformanimator.h"
#include "undoredo.h"
#include "boxeslist.h"
#include <QDebug>

TransformAnimator::TransformAnimator() : ComplexAnimator()
{
    setName("transformation");
    mScaleAnimator.setName("scale");
    mScaleAnimator.setCurrentValue(QPointF(1., 1.));
    mRotAnimator.setName("rot");
    mRotAnimator.setCurrentValue(0.);
    mPosAnimator.setName("trans");
    mPosAnimator.setCurrentValue(QPointF(0., 0.) );
    mPivotAnimator.setName("pivot");
    mPivotAnimator.setCurrentValue(QPointF(0., 0.) );
    mOpacityAnimator.setName("opacity");
    mOpacityAnimator.setValueRange(0., 100.);
    mOpacityAnimator.setPrefferedValueStep(5.);
    mOpacityAnimator.setCurrentValue(100.);
    mOpacityAnimator.freezeMinMaxValues();

    mPosAnimator.blockPointer();
    mRotAnimator.blockPointer();
    mScaleAnimator.blockPointer();
    mPivotAnimator.blockPointer();
    mOpacityAnimator.blockPointer();

    addChildAnimator(&mPosAnimator);
    addChildAnimator(&mRotAnimator);
    addChildAnimator(&mScaleAnimator);
    mScaleAnimator.setPrefferedValueStep(0.05);
    addChildAnimator(&mPivotAnimator);
    addChildAnimator(&mOpacityAnimator);
}

#include <QSqlError>
int TransformAnimator::saveToSql() {
    int posAnimatorId = mPosAnimator.saveToSql();
    int scaleAnimatorId = mScaleAnimator.saveToSql();
    int pivotAnimatorId = mPivotAnimator.saveToSql();
    int rotAnimatorId = mRotAnimator.saveToSql();
    int opacityAnimatorId = mOpacityAnimator.saveToSql();
    QSqlQuery query;
    if(!query.exec(
        QString("INSERT INTO transformanimator (posanimatorid, scaleanimatorid, "
                "pivotanimatorid, rotanimatorid, opacityanimatorid ) "
                "VALUES (%1, %2, %3, %4, %5)").
                arg(posAnimatorId).
                arg(scaleAnimatorId).
                arg(pivotAnimatorId).
                arg(rotAnimatorId).
                arg(opacityAnimatorId) ) ) {
        qDebug() << query.lastError() << endl << query.lastQuery();
    }

    return query.lastInsertId().toInt();
}

void TransformAnimator::loadFromSql(int transformAnimatorId) {
    QSqlQuery query;

    QString queryStr = "SELECT * FROM transformanimator WHERE id = " +
            QString::number(transformAnimatorId);
    if(query.exec(queryStr)) {
        query.next();
        int posanimatorid = query.record().indexOf("posanimatorid");
        int scaleanimatorid = query.record().indexOf("scaleanimatorid");
        int pivotanimatorid = query.record().indexOf("pivotanimatorid");
        int rotanimatorid = query.record().indexOf("rotanimatorid");
        int opacityanimatorid = query.record().indexOf("opacityanimatorid");

        //loadKeysFromSql(qrealAnimatorId);

        mPosAnimator.loadFromSql(posanimatorid);
        mScaleAnimator.loadFromSql(scaleanimatorid);
        mPivotAnimator.loadFromSql(pivotanimatorid);
        mRotAnimator.loadFromSql(rotanimatorid);
        mOpacityAnimator.loadFromSql(opacityanimatorid);
    } else {
        qDebug() << "Could not load qpointfanimator with id " << transformAnimatorId;
    }
}

void TransformAnimator::copyTransformationTo(
                                        TransformAnimator *targetAnimator) {
    QPointF currScale = mScaleAnimator.getCurrentValue();
    QPointF currTrans = mPosAnimator.getCurrentValue();
    QPointF currPivot = mPivotAnimator.getCurrentValue();
    qreal currRot = mRotAnimator.getCurrentValue();
    qreal currOpacity = mOpacityAnimator.getCurrentValue();

    targetAnimator->setScale(currScale.x(), currScale.y() );
    targetAnimator->setPosition(currTrans.x(), currTrans.y() );
    targetAnimator->setRotation(currRot);
    targetAnimator->setOpacity(currOpacity);
    targetAnimator->setPivot(currPivot);
}

void TransformAnimator::resetScale()
{
    mScaleAnimator.setCurrentValue(QPointF(1., 1.) );
}

void TransformAnimator::resetTranslation()
{
    mPosAnimator.setCurrentValue(QPointF(0., 0.) );
}

void TransformAnimator::resetRotation()
{
    mRotAnimator.setCurrentValue(0.);
}


void TransformAnimator::reset()
{
    resetScale();
    resetTranslation();
    resetRotation();
}

void TransformAnimator::rotateRelativeToSavedValue(qreal rotRel) {
    mRotAnimator.retrieveSavedValue();
    mRotAnimator.incCurrentValue(rotRel);
}

void TransformAnimator::rotateRelativeToSavedValue(qreal rotRel, QPointF pivot) {
    QMatrix matrix;
    matrix.translate(pivot.x(),
                     pivot.y());
    matrix.rotate(rotRel);
    matrix.translate(-pivot.x() + mPosAnimator.getSavedXValue(),
                     -pivot.y() + mPosAnimator.getSavedYValue() );
    rotateRelativeToSavedValue(rotRel);
    mPosAnimator.setCurrentValue(QPointF(matrix.dx(), matrix.dy()) );
}

void TransformAnimator::moveRelativeToSavedValue(qreal dX, qreal dY) {
    mPosAnimator.retrieveSavedValue();
    mPosAnimator.incCurrentValue(dX, dY);
}

void TransformAnimator::translate(qreal dX, qreal dY)
{
    mPosAnimator.incCurrentValue(dX, dY);
}

void TransformAnimator::scale(qreal sx, qreal sy)
{
    mScaleAnimator.retrieveSavedValue();
    mScaleAnimator.multCurrentValue(sx, sy);
}

void TransformAnimator::scale(qreal sx, qreal sy, QPointF pivot)
{
    QMatrix matrix;
    matrix.translate(pivot.x(),
                     pivot.y());
    matrix.scale(sx, sy);
    matrix.translate(-pivot.x() + mPosAnimator.getSavedXValue(),
                     -pivot.y() + mPosAnimator.getSavedYValue() );
    scale(sx, sy);
    mPosAnimator.setCurrentValue(QPointF(matrix.dx(), matrix.dy()) );
}

void TransformAnimator::startOpacityTransform() {
    mOpacityAnimator.startTransform();
}

void TransformAnimator::setOpacity(qreal newOpacity) {
    mOpacityAnimator.setCurrentValue(newOpacity);
}

void TransformAnimator::setScale(qreal sx, qreal sy)
{
    mScaleAnimator.setCurrentValue(QPointF(sx, sy) );
}

void TransformAnimator::setPosition(qreal x, qreal y)
{
    mPosAnimator.setCurrentValue(QPointF(x, y) );
}

void TransformAnimator::setRotation(qreal rot)
{
    mRotAnimator.setCurrentValue(rot);
}

void TransformAnimator::startRotTransform()
{
    mRotAnimator.startTransform();
}

void TransformAnimator::startPosTransform() {
    mPosAnimator.startTransform();
}

void TransformAnimator::startScaleTransform() {
    mScaleAnimator.startTransform();
}

qreal TransformAnimator::getYScale()
{
    return mScaleAnimator.getYValue();
}

qreal TransformAnimator::getXScale()
{
    return mScaleAnimator.getXValue();
}

void TransformAnimator::setPivotWithoutChangingTransformation(qreal x, qreal y)
{
    setPivotWithoutChangingTransformation(QPointF(x, y) );
}

void TransformAnimator::setPivotWithoutChangingTransformation(QPointF point, bool finish)
{
    QMatrix currentMatrix;
    qreal pivotX = mPivotAnimator.getXValue();
    qreal pivotY = mPivotAnimator.getYValue();
    currentMatrix.translate(pivotX + mPosAnimator.getXValue(),
                     pivotY + mPosAnimator.getYValue());

    currentMatrix.rotate(mRotAnimator.getCurrentValue() );
    currentMatrix.scale(mScaleAnimator.getXValue(),
                        mScaleAnimator.getYValue() );

    currentMatrix.translate(-pivotX,
                            -pivotY);

    QMatrix futureMatrix;
    futureMatrix.translate(point.x() + mPosAnimator.getXValue(),
                     point.y() + mPosAnimator.getYValue());

    futureMatrix.rotate(mRotAnimator.getCurrentValue() );
    futureMatrix.scale(mScaleAnimator.getXValue(),
                       mScaleAnimator.getYValue() );

    futureMatrix.translate(-point.x(),
                            -point.y());

    mPosAnimator.incAllValues(currentMatrix.dx() - futureMatrix.dx(),
                              currentMatrix.dy() - futureMatrix.dy());

    mPivotAnimator.setCurrentValue(point, finish);

    callUpdater();
}

void TransformAnimator::setPivot(QPointF point, bool finish) {

    mPivotAnimator.setCurrentValue(point, finish);

    callUpdater();
}

QPointF TransformAnimator::getPivot()
{
    return mPivotAnimator.getCurrentValue();
}

qreal TransformAnimator::dx()
{
    return mPosAnimator.getXValue();
}

qreal TransformAnimator::dy()
{
    return mPosAnimator.getYValue();
}

qreal TransformAnimator::rot()
{
    return mRotAnimator.getCurrentValue();
}

qreal TransformAnimator::xScale()
{
    return mScaleAnimator.getXValue();
}

qreal TransformAnimator::yScale()
{
    return mScaleAnimator.getYValue();
}

QPointF TransformAnimator::pos()
{
    return mPosAnimator.getCurrentValue();
}

qreal TransformAnimator::getPivotX()
{
    return mPivotAnimator.getXValue();
}

qreal TransformAnimator::getPivotY()
{
    return mPivotAnimator.getYValue();
}

qreal TransformAnimator::getOpacity()
{
    return mOpacityAnimator.getCurrentValue();
}

QMatrix TransformAnimator::getCurrentValue()
{
    QMatrix matrix;
    qreal pivotX = mPivotAnimator.getXValue();
    qreal pivotY = mPivotAnimator.getYValue();
    matrix.translate(pivotX + mPosAnimator.getXValue(),
                     pivotY + mPosAnimator.getYValue());

    matrix.rotate(mRotAnimator.getCurrentValue() );
    matrix.scale(mScaleAnimator.getXValue(), mScaleAnimator.getYValue() );

    matrix.translate(-pivotX,
                     -pivotY);
    return matrix;
}
