#include "Animators/transformanimator.h"
#include "undoredo.h"
#include <QDebug>

TransformAnimator::TransformAnimator() : ComplexAnimator()
{
    prp_setName("transformation");
    mScaleAnimator->prp_setName("scale");
    mScaleAnimator->qra_setCurrentValue(QPointF(1., 1.));
    mScaleAnimator->setPrefferedValueStep(0.05);

    mRotAnimator->prp_setName("rot");
    mRotAnimator->qra_setCurrentValue(0.);
    mPosAnimator->prp_setName("trans");
    mPosAnimator->qra_setCurrentValue(QPointF(0., 0.) );
    mPivotAnimator->prp_setName("pivot");
    mPivotAnimator->qra_setCurrentValue(QPointF(0., 0.) );
    mOpacityAnimator->prp_setName("opacity");
    mOpacityAnimator->qra_setValueRange(0., 100.);
    mOpacityAnimator->setPrefferedValueStep(5.);
    mOpacityAnimator->qra_setCurrentValue(100.);
    mOpacityAnimator->freezeMinMaxValues();

    ca_addChildAnimator(mPosAnimator.data());
    ca_addChildAnimator(mRotAnimator.data());
    ca_addChildAnimator(mScaleAnimator.data());
    ca_addChildAnimator(mPivotAnimator.data());
    ca_addChildAnimator(mOpacityAnimator.data());
}

#include <QSqlError>
int TransformAnimator::prp_saveToSql(QSqlQuery *query, const int &parentId) {
    Q_UNUSED(parentId);
    int posAnimatorId = mPosAnimator->prp_saveToSql(query);
    int scaleAnimatorId = mScaleAnimator->prp_saveToSql(query);
    int pivotAnimatorId = mPivotAnimator->prp_saveToSql(query);
    int rotAnimatorId = mRotAnimator->prp_saveToSql(query);
    int opacityAnimatorId = mOpacityAnimator->prp_saveToSql(query);
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

void TransformAnimator::prp_loadFromSql(const int &transformAnimatorId) {
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

        mPosAnimator->prp_loadFromSql(query.value(posanimatorid).toInt());
        mScaleAnimator->prp_loadFromSql(query.value(scaleanimatorid).toInt());
        mPivotAnimator->prp_loadFromSql(query.value(pivotanimatorid).toInt());
        mRotAnimator->prp_loadFromSql(query.value(rotanimatorid).toInt());
        mOpacityAnimator->prp_loadFromSql(query.value(opacityanimatorid).toInt());
    } else {
        qDebug() << "Could not load qpointfanimator with id " << transformAnimatorId;
    }
}

void TransformAnimator::copyTransformationTo(
                                        TransformAnimator *targetAnimator) {
    QPointF currScale = mScaleAnimator->qra_getCurrentValue();
    QPointF currTrans = mPosAnimator->qra_getCurrentValue();
    QPointF currPivot = mPivotAnimator->qra_getCurrentValue();
    qreal currRot = mRotAnimator->qra_getCurrentValue();
    qreal currOpacity = mOpacityAnimator->qra_getCurrentValue();

    targetAnimator->setScale(currScale.x(), currScale.y() );
    targetAnimator->setPosition(currTrans.x(), currTrans.y() );
    targetAnimator->setRotation(currRot);
    targetAnimator->setOpacity(currOpacity);
    targetAnimator->setPivot(currPivot);
}

void TransformAnimator::resetScale(bool finish)
{
    mScaleAnimator->qra_setCurrentValue(QPointF(1., 1.), finish);
}

void TransformAnimator::resetTranslation(bool finish)
{
    mPosAnimator->qra_setCurrentValue(QPointF(0., 0.), finish);
}

void TransformAnimator::resetRotation(bool finish)
{
    mRotAnimator->qra_setCurrentValue(0., finish);
}


void TransformAnimator::reset(bool finish)
{
    resetScale(finish);
    resetTranslation(finish);
    resetRotation(finish);
}

void TransformAnimator::rotateRelativeToSavedValue(qreal rotRel) {
    mRotAnimator->incSavedValueToCurrentValue(rotRel);
}

void TransformAnimator::rotateRelativeToSavedValue(qreal rotRel,
                                                   QPointF pivot) {
    QMatrix matrix;
    matrix.translate(pivot.x(),
                     pivot.y());
    matrix.rotate(rotRel);
    matrix.translate(-pivot.x() + mPosAnimator->getSavedXValue(),
                     -pivot.y() + mPosAnimator->getSavedYValue() );
    rotateRelativeToSavedValue(rotRel);
    mPosAnimator->qra_setCurrentValue(QPointF(matrix.dx(), matrix.dy()) );
}

void TransformAnimator::moveRelativeToSavedValue(qreal dX, qreal dY) {
    mPosAnimator->incSavedValueToCurrentValue(dX, dY);
}

void TransformAnimator::translate(qreal dX, qreal dY) {
    mPosAnimator->qra_incCurrentValue(dX, dY);
}

void TransformAnimator::scale(qreal sx, qreal sy)
{
    mScaleAnimator->multSavedValueToCurrentValue(sx, sy);
}

void TransformAnimator::scaleRelativeToSavedValue(qreal sx, qreal sy,
                                                  QPointF pivot)
{
    QMatrix matrix;

    matrix.translate(pivot.x(),
                     pivot.y());
    matrix.rotate(mRotAnimator->qra_getCurrentValue());
    matrix.scale(sx, sy);
    matrix.rotate(-mRotAnimator->qra_getCurrentValue());
    matrix.translate(-pivot.x() + mPosAnimator->getSavedXValue(),
                     -pivot.y() + mPosAnimator->getSavedYValue() );

    scale(sx, sy);
    mPosAnimator->qra_setCurrentValue(QPointF(matrix.dx(), matrix.dy()) );
}

void TransformAnimator::startOpacityTransform() {
    mOpacityAnimator->prp_startTransform();
}

void TransformAnimator::setOpacity(qreal newOpacity) {
    mOpacityAnimator->qra_setCurrentValue(newOpacity);
}

void TransformAnimator::setScale(qreal sx, qreal sy)
{
    mScaleAnimator->qra_setCurrentValue(QPointF(sx, sy) );
}

void TransformAnimator::setPosition(qreal x, qreal y)
{
    mPosAnimator->qra_setCurrentValue(QPointF(x, y) );
}

void TransformAnimator::setRotation(qreal rot)
{
    mRotAnimator->qra_setCurrentValue(rot);
}

void TransformAnimator::startRotTransform()
{
    mRotAnimator->prp_startTransform();
}

void TransformAnimator::startPosTransform() {
    mPosAnimator->prp_startTransform();
}

void TransformAnimator::startScaleTransform() {
    mScaleAnimator->prp_startTransform();
}

qreal TransformAnimator::getYScale()
{
    return mScaleAnimator->getYValue();
}

qreal TransformAnimator::getXScale()
{
    return mScaleAnimator->getXValue();
}

void TransformAnimator::setPivotWithoutChangingTransformation(qreal x, qreal y)
{
    setPivotWithoutChangingTransformation(QPointF(x, y) );
}

void TransformAnimator::setPivotWithoutChangingTransformation(QPointF point, bool finish)
{
    if(!mPivotAnimator->prp_isRecording() && !mPosAnimator->prp_isRecording() &&
       !mRotAnimator->prp_isRecording() && !mScaleAnimator->prp_isRecording()) {
        QMatrix currentMatrix;
        qreal pivotX = mPivotAnimator->getXValue();
        qreal pivotY = mPivotAnimator->getYValue();
        currentMatrix.translate(pivotX + mPosAnimator->getXValue(),
                         pivotY + mPosAnimator->getYValue());

        currentMatrix.rotate(mRotAnimator->qra_getCurrentValue() );
        currentMatrix.scale(mScaleAnimator->getXValue(),
                            mScaleAnimator->getYValue() );

        currentMatrix.translate(-pivotX,
                                -pivotY);

        QMatrix futureMatrix;
        futureMatrix.translate(point.x() + mPosAnimator->getXValue(),
                         point.y() + mPosAnimator->getYValue());

        futureMatrix.rotate(mRotAnimator->qra_getCurrentValue() );
        futureMatrix.scale(mScaleAnimator->getXValue(),
                           mScaleAnimator->getYValue() );

        futureMatrix.translate(-point.x(),
                                -point.y());


        mPosAnimator->qra_incAllValues(currentMatrix.dx() - futureMatrix.dx(),
                                  currentMatrix.dy() - futureMatrix.dy());
    }
//    if(!mPivotAnimator->isRecording()) {
//        mPosAnimator->incAllValues(currentMatrix.dx() - futureMatrix.dx(),
//                                  currentMatrix.dy() - futureMatrix.dy());
//    } else {
//    mPosAnimator->incCurrentValue(currentMatrix.dx() - futureMatrix.dx(),
//                                 currentMatrix.dy() - futureMatrix.dy());
//    }

    mPivotAnimator->qra_setCurrentValue(point, finish);

    //callUpdater();
}

void TransformAnimator::setPivot(QPointF point, bool finish) {

    mPivotAnimator->qra_setCurrentValue(point, finish);

    //callUpdater();
}

QPointF TransformAnimator::getPivot()
{
    return mPivotAnimator->qra_getCurrentValue();
}

qreal TransformAnimator::dx()
{
    return mPosAnimator->getXValue();
}

qreal TransformAnimator::dy()
{
    return mPosAnimator->getYValue();
}

qreal TransformAnimator::rot()
{
    return mRotAnimator->qra_getCurrentValue();
}

qreal TransformAnimator::xScale()
{
    return mScaleAnimator->getXValue();
}

qreal TransformAnimator::yScale()
{
    return mScaleAnimator->getYValue();
}

QPointF TransformAnimator::pos()
{
    return mPosAnimator->qra_getCurrentValue();
}

qreal TransformAnimator::getPivotX()
{
    return mPivotAnimator->getXValue();
}

qreal TransformAnimator::getPivotY()
{
    return mPivotAnimator->getYValue();
}

qreal TransformAnimator::getOpacity()
{
    return mOpacityAnimator->qra_getCurrentValue();
}

QMatrix TransformAnimator::getCurrentTransformationMatrix() {
    QMatrix matrix;// = mBaseTransformation;
    qreal pivotX = mPivotAnimator->getXValue();
    qreal pivotY = mPivotAnimator->getYValue();
//    if(mBaseTransformationSet) {
//        matrix.translate(mBaseTransformation.dx(),
//                         mBaseTransformation.dy());
//    }
    matrix.translate(pivotX + mPosAnimator->getXValue(),
                     pivotY + mPosAnimator->getYValue());

    matrix.rotate(mRotAnimator->qra_getCurrentValue() );
    matrix.scale(mScaleAnimator->getXValue(),
                 mScaleAnimator->getYValue() );

    matrix.translate(-pivotX,
                     -pivotY);
//    if(mBaseTransformationSet) {
//        return mBaseTransformation*matrix;
//    } else {
        return matrix;
//    }
}

void TransformAnimator::prp_makeDuplicate(Property *target) {
    TransformAnimator *transformPtr = (TransformAnimator*)target;

    transformPtr->duplicatePivotAnimatorFrom(mPivotAnimator.data());
    transformPtr->duplicatePosAnimatorFrom(mPosAnimator.data());
    transformPtr->duplicateScaleAnimatorFrom(mScaleAnimator.data());
    transformPtr->duplicateRotAnimatorFrom(mRotAnimator.data());
    transformPtr->duplicateOpacityAnimatorFrom(mOpacityAnimator.data());
}

void TransformAnimator::duplicatePivotAnimatorFrom(
        QPointFAnimator *source) {
    source->prp_makeDuplicate(mPivotAnimator.data());
}

void TransformAnimator::duplicatePosAnimatorFrom(
        QPointFAnimator *source) {
    source->prp_makeDuplicate(mPosAnimator.data());
}

void TransformAnimator::duplicateScaleAnimatorFrom(
        QPointFAnimator *source) {
    source->prp_makeDuplicate(mScaleAnimator.data());
}

void TransformAnimator::duplicateRotAnimatorFrom(
        QrealAnimator *source) {
    source->prp_makeDuplicate(mRotAnimator.data());
}

void TransformAnimator::duplicateOpacityAnimatorFrom(
        QrealAnimator *source) {
    source->prp_makeDuplicate(mOpacityAnimator.data());
}

void TransformAnimator::moveByAbs(const QMatrix &combinedTrans,
                                  const QPointF &absTrans) {
    moveToAbs(combinedTrans,
              combinedTrans.map(mPosAnimator->qra_getSavedValue()) +
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
    mPosAnimator->qra_setCurrentValue(relPos, saveUndoRedo);
}
