#include "Animators/transformanimator.h"
#include "undoredo.h"
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
int TransformAnimator::saveToSql(QSqlQuery *query) {
    int posAnimatorId = mPosAnimator.saveToSql(query);
    int scaleAnimatorId = mScaleAnimator.saveToSql(query);
    int pivotAnimatorId = mPivotAnimator.saveToSql(query);
    int rotAnimatorId = mRotAnimator.saveToSql(query);
    int opacityAnimatorId = mOpacityAnimator.saveToSql(query);
    if(!query->exec(
        QString("INSERT INTO transformanimator (posanimatorid, scaleanimatorid, "
                "pivotanimatorid, rotanimatorid, opacityanimatorid ) "
                "VALUES (%1, %2, %3, %4, %5)").
                arg(posAnimatorId).
                arg(scaleAnimatorId).
                arg(pivotAnimatorId).
                arg(rotAnimatorId).
                arg(opacityAnimatorId) ) ) {
        qDebug() << query->lastError() << endl << query->lastQuery();
    }

    return query->lastInsertId().toInt();
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

        mPosAnimator.loadFromSql(query.value(posanimatorid).toInt());
        mScaleAnimator.loadFromSql(query.value(scaleanimatorid).toInt());
        mPivotAnimator.loadFromSql(query.value(pivotanimatorid).toInt());
        mRotAnimator.loadFromSql(query.value(rotanimatorid).toInt());
        mOpacityAnimator.loadFromSql(query.value(opacityanimatorid).toInt());
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

void TransformAnimator::resetScale(bool finish)
{
    mScaleAnimator.setCurrentValue(QPointF(1., 1.), finish);
}

void TransformAnimator::resetTranslation(bool finish)
{
    mPosAnimator.setCurrentValue(QPointF(0., 0.), finish);
}

void TransformAnimator::resetRotation(bool finish)
{
    mRotAnimator.setCurrentValue(0., finish);
}


void TransformAnimator::reset(bool finish)
{
    resetScale(finish);
    resetTranslation(finish);
    resetRotation(finish);
}

void TransformAnimator::rotateRelativeToSavedValue(qreal rotRel) {
    mRotAnimator.incSavedValueToCurrentValue(rotRel);
}

void TransformAnimator::rotateRelativeToSavedValue(qreal rotRel,
                                                   QPointF pivot) {
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
    mPosAnimator.incSavedValueToCurrentValue(dX, dY);
}

void TransformAnimator::translate(qreal dX, qreal dY) {
    mPosAnimator.incCurrentValue(dX, dY);
}

void TransformAnimator::scale(qreal sx, qreal sy)
{
    mScaleAnimator.multSavedValueToCurrentValue(sx, sy);
}

void TransformAnimator::scaleRelativeToSavedValue(qreal sx, qreal sy,
                                                  QPointF pivot)
{
    QMatrix matrix;

    matrix.translate(pivot.x(),
                     pivot.y());
    matrix.rotate(mRotAnimator.getCurrentValue());
    matrix.scale(sx, sy);
    matrix.rotate(-mRotAnimator.getCurrentValue());
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
    if(!mPivotAnimator.isRecording() && !mPosAnimator.isRecording() &&
       !mRotAnimator.isRecording() && !mScaleAnimator.isRecording()) {
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
    }
//    if(!mPivotAnimator.isRecording()) {
//        mPosAnimator.incAllValues(currentMatrix.dx() - futureMatrix.dx(),
//                                  currentMatrix.dy() - futureMatrix.dy());
//    } else {
//    mPosAnimator.incCurrentValue(currentMatrix.dx() - futureMatrix.dx(),
//                                 currentMatrix.dy() - futureMatrix.dy());
//    }

    mPivotAnimator.setCurrentValue(point, finish);

    //callUpdater();
}

void TransformAnimator::setPivot(QPointF point, bool finish) {

    mPivotAnimator.setCurrentValue(point, finish);

    //callUpdater();
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

QMatrix TransformAnimator::getCurrentValue() {
    QMatrix matrix;// = mBaseTransformation;
    qreal pivotX = mPivotAnimator.getXValue();
    qreal pivotY = mPivotAnimator.getYValue();
//    if(mBaseTransformationSet) {
//        matrix.translate(mBaseTransformation.dx(),
//                         mBaseTransformation.dy());
//    }
    matrix.translate(pivotX + mPosAnimator.getXValue(),
                     pivotY + mPosAnimator.getYValue());

    matrix.rotate(mRotAnimator.getCurrentValue() );
    matrix.scale(mScaleAnimator.getXValue(),
                 mScaleAnimator.getYValue() );

    matrix.translate(-pivotX,
                     -pivotY);
//    if(mBaseTransformationSet) {
//        return mBaseTransformation*matrix;
//    } else {
        return matrix;
//    }
}

void TransformAnimator::makeDuplicate(QrealAnimator *target) {
    TransformAnimator *transformPtr = (TransformAnimator*)target;

    transformPtr->duplicatePivotAnimatorFrom(&mPivotAnimator);
    transformPtr->duplicatePosAnimatorFrom(&mPosAnimator);
    transformPtr->duplicateScaleAnimatorFrom(&mScaleAnimator);
    transformPtr->duplicateRotAnimatorFrom(&mRotAnimator);
    transformPtr->duplicateOpacityAnimatorFrom(&mOpacityAnimator);
}

void TransformAnimator::duplicatePivotAnimatorFrom(
        QPointFAnimator *source) {
    source->makeDuplicate(&mPivotAnimator);
}

void TransformAnimator::duplicatePosAnimatorFrom(
        QPointFAnimator *source) {
    source->makeDuplicate(&mPosAnimator);
}

void TransformAnimator::duplicateScaleAnimatorFrom(
        QPointFAnimator *source) {
    source->makeDuplicate(&mScaleAnimator);
}

void TransformAnimator::duplicateRotAnimatorFrom(
        QrealAnimator *source) {
    source->makeDuplicate(&mRotAnimator);
}

void TransformAnimator::duplicateOpacityAnimatorFrom(
        QrealAnimator *source) {
    source->makeDuplicate(&mOpacityAnimator);
}

void TransformAnimator::moveByAbs(const QMatrix &combinedTrans,
                                  const QPointF &absTrans) {
    moveToAbs(combinedTrans,
              combinedTrans.map(mPosAnimator.getSavedValue()) +
              absTrans);
}

void TransformAnimator::moveToAbs(const QMatrix &combinedTrans,
                                  QPointF absPos) {
    setAbsolutePos(combinedTrans, absPos, false);
}

void TransformAnimator::setAbsolutePos(const QMatrix &combinedTrans,
                                       QPointF pos,
                                       bool saveUndoRedo) {
    QPointF newPos = combinedTrans.inverted().map(pos);
    setRelativePos(newPos, saveUndoRedo);
}

void TransformAnimator::setRelativePos(QPointF relPos,
                                       bool saveUndoRedo) {
    mPosAnimator.setCurrentValue(relPos, saveUndoRedo);
}
