#include "Animators/transformanimator.h"
#include "undoredo.h"
#include <QDebug>
#include "boxpathpoint.h"

void BasicTransformAnimator::resetScale(const bool &finish) {
    mScaleAnimator->setCurrentPointValue(QPointF(1., 1.), finish);
}

void BasicTransformAnimator::resetTranslation(const bool &finish) {
    mPosAnimator->setCurrentPointValue(QPointF(0., 0.), finish);
}

void BasicTransformAnimator::resetRotation(const bool &finish) {
    mRotAnimator->qra_setCurrentValue(0., finish);
}

void BasicTransformAnimator::reset(const bool &finish) {
    resetScale(finish);
    resetTranslation(finish);
    resetRotation(finish);
}

void BasicTransformAnimator::setScale(const qreal &sx, const qreal &sy) {
    mScaleAnimator->setCurrentPointValue(QPointF(sx, sy) );
}

void BasicTransformAnimator::setPosition(const qreal &x, const qreal &y) {
    mPosAnimator->setCurrentPointValue(QPointF(x, y) );
}

void BasicTransformAnimator::setRotation(const qreal &rot) {
    mRotAnimator->qra_setCurrentValue(rot);
}

void BasicTransformAnimator::startRotTransform() {
    mRotAnimator->prp_startTransform();
}

void BasicTransformAnimator::startPosTransform() {
    mPosAnimator->prp_startTransform();
}

void BasicTransformAnimator::startScaleTransform() {
    mScaleAnimator->prp_startTransform();
}

qreal BasicTransformAnimator::getYScale() {
    return mScaleAnimator->getYValue();
}

qreal BasicTransformAnimator::getXScale() {
    return mScaleAnimator->getXValue();
}

void BasicTransformAnimator::rotateRelativeToSavedValue(const qreal &rotRel) {
    mRotAnimator->incSavedValueToCurrentValue(rotRel);
}

void BasicTransformAnimator::moveRelativeToSavedValue(const qreal &dX,
                                                      const qreal &dY) {
    mPosAnimator->incSavedValueToCurrentValue(dX, dY);
}

void BasicTransformAnimator::translate(const qreal &dX, const qreal &dY) {
    mPosAnimator->incCurrentValues(dX, dY);
}

void BasicTransformAnimator::scale(const qreal &sx, const qreal &sy) {
    mScaleAnimator->multSavedValueToCurrentValue(sx, sy);
}

qreal BasicTransformAnimator::dx() {
    return mPosAnimator->getXValue();
}

qreal BasicTransformAnimator::dy() {
    return mPosAnimator->getYValue();
}

qreal BasicTransformAnimator::rot() {
    return mRotAnimator->qra_getCurrentValue();
}

qreal BasicTransformAnimator::xScale() {
    return mScaleAnimator->getXValue();
}

qreal BasicTransformAnimator::yScale() {
    return mScaleAnimator->getYValue();
}

QPointF BasicTransformAnimator::pos() {
    return mPosAnimator->getCurrentPointValue();
}

QMatrix BasicTransformAnimator::getCurrentTransformationMatrix() {
    QMatrix matrix;

    matrix.translate(mPosAnimator->getXValue(),
                     mPosAnimator->getYValue());

    matrix.rotate(mRotAnimator->qra_getCurrentValue() );
    matrix.scale(mScaleAnimator->getXValue(),
                 mScaleAnimator->getYValue() );
    return matrix;
}

void BasicTransformAnimator::duplicatePosAnimatorFrom(
                        QPointFAnimator *source) {
    source->makeDuplicate(mPosAnimator.data());
}

void BasicTransformAnimator::duplicateScaleAnimatorFrom(
                        QPointFAnimator *source) {
    source->makeDuplicate(mScaleAnimator.data());
}

void BasicTransformAnimator::duplicateRotAnimatorFrom(
                        QrealAnimator *source) {
    source->makeDuplicate(mRotAnimator.data());
}

void BasicTransformAnimator::moveByAbs(
                        const QMatrix &combinedTrans,
                        const QPointF &absTrans) {
    moveToAbs(combinedTrans,
              combinedTrans.map(mPosAnimator->getSavedPointValue()) +
              absTrans);
}

void BasicTransformAnimator::moveToAbs(
                        const QMatrix &combinedTrans,
                        const QPointF &absPos) {
    setAbsolutePos(combinedTrans, absPos, false);
}

void BasicTransformAnimator::setAbsolutePos(
                        const QMatrix &combinedTrans,
                        const QPointF &pos,
                        const bool &saveUndoRedo) {
    QPointF newPos = combinedTrans.inverted().map(pos);
    setRelativePos(newPos, saveUndoRedo);
}

void BasicTransformAnimator::setRelativePos(const QPointF &relPos,
                                            const bool &saveUndoRedo) {
    mPosAnimator->setCurrentPointValue(relPos, saveUndoRedo);
}

void BasicTransformAnimator::rotateRelativeToSavedValue(const qreal &rotRel,
                                                        const QPointF &pivot) {
    QMatrix matrix;
    matrix.translate(pivot.x(),
                     pivot.y());
    matrix.rotate(rotRel);
    matrix.translate(-pivot.x() + mPosAnimator->getSavedXValue(),
                     -pivot.y() + mPosAnimator->getSavedYValue() );
    rotateRelativeToSavedValue(rotRel);
    mPosAnimator->setCurrentPointValue(QPointF(matrix.dx(), matrix.dy()) );
}

void BasicTransformAnimator::scaleRelativeToSavedValue(const qreal &sx,
                                                      const qreal &sy,
                                                      const QPointF &pivot) {
    QMatrix matrix;

    matrix.translate(pivot.x(),
                     pivot.y());
    matrix.rotate(mRotAnimator->qra_getCurrentValue());
    matrix.scale(sx, sy);
    matrix.rotate(-mRotAnimator->qra_getCurrentValue());
    matrix.translate(-pivot.x() + mPosAnimator->getSavedXValue(),
                     -pivot.y() + mPosAnimator->getSavedYValue() );

    scale(sx, sy);
    mPosAnimator->setCurrentPointValue(QPointF(matrix.dx(), matrix.dy()) );
}

void BasicTransformAnimator::makeDuplicate(BasicTransformAnimator *target) {
    target->duplicatePosAnimatorFrom(mPosAnimator.data());
    target->duplicateScaleAnimatorFrom(mScaleAnimator.data());
    target->duplicateRotAnimatorFrom(mRotAnimator.data());
}

TransformAnimator::TransformAnimator(BoundingBox *parent) :
    BasicTransformAnimator() {
    mPivotAnimator = (new BoxPathPoint(parent))->ref<MovablePoint>();

    prp_setName("transformation");
    mScaleAnimator->prp_setName("scale");
    mScaleAnimator->setCurrentPointValue(QPointF(1., 1.));
    mScaleAnimator->setPrefferedValueStep(0.05);

    mRotAnimator->prp_setName("rot");
    mRotAnimator->qra_setCurrentValue(0.);
    mPosAnimator->prp_setName("trans");
    mPosAnimator->setCurrentPointValue(QPointF(0., 0.) );
    mPivotAnimator->prp_setName("pivot");
    mPivotAnimator->setCurrentPointValue(QPointF(0., 0.) );
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

MovablePoint *TransformAnimator::getPivotMovablePoint() {
    return mPivotAnimator.data();
}

#include <QSqlError>
int TransformAnimator::prp_saveToSql(QSqlQuery *query,
                                     const int &parentId) {
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

void TransformAnimator::resetPivot(const bool &finish) {
    mPivotAnimator->setCurrentPointValue(QPointF(0., 0.), finish);
}

void TransformAnimator::reset(const bool &finish) {
    BasicTransformAnimator::reset(finish);
    resetPivot(finish);
}

void TransformAnimator::startOpacityTransform() {
    mOpacityAnimator->prp_startTransform();
}

void TransformAnimator::setOpacity(const qreal &newOpacity) {
    mOpacityAnimator->qra_setCurrentValue(newOpacity);
}

void TransformAnimator::pivotTransformStarted() {
    if(!mPosAnimator->prp_isDescendantRecording()) {
        mPosAnimator->prp_startTransform();
    }
}

void TransformAnimator::pivotTransformFinished() {
    if(!mPosAnimator->prp_isDescendantRecording()) {
        mPosAnimator->prp_finishTransform();
    }
}

void TransformAnimator::setPivotWithoutChangingTransformation(
                                QPointF point, const bool &finish) {
    if(mPosAnimator->prp_isDescendantRecording()) {
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
                           mScaleAnimator->getYValue());

        futureMatrix.translate(-point.x(),
                               -point.y());


        point += QPointF(currentMatrix.dx() - futureMatrix.dx(),
                         currentMatrix.dy() - futureMatrix.dy());

    } else {
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


        mPosAnimator->incAllValues(currentMatrix.dx() - futureMatrix.dx(),
                                       currentMatrix.dy() - futureMatrix.dy());
    }
    mPivotAnimator->setCurrentPointValue(point, finish);
}

void TransformAnimator::setPivot(const QPointF &point,
                                 const bool &finish) {
    mPivotAnimator->setCurrentPointValue(point, finish);

    //callUpdater();
}

QPointF TransformAnimator::getPivot() {
    return mPivotAnimator->getCurrentPointValue();
}

qreal TransformAnimator::getPivotX() {
    return mPivotAnimator->getXValue();
}

qreal TransformAnimator::getPivotY() {
    return mPivotAnimator->getYValue();
}

qreal TransformAnimator::getOpacity() {
    return mOpacityAnimator->qra_getCurrentValue();
}

QMatrix TransformAnimator::getCurrentTransformationMatrix() {
    QMatrix matrix;
    qreal pivotX = mPivotAnimator->getXValue();
    qreal pivotY = mPivotAnimator->getYValue();

    matrix.translate(pivotX + mPosAnimator->getXValue(),
                     pivotY + mPosAnimator->getYValue());

    matrix.rotate(mRotAnimator->qra_getCurrentValue() );
    matrix.scale(mScaleAnimator->getXValue(),
                 mScaleAnimator->getYValue() );

    matrix.translate(-pivotX,
                     -pivotY);
    return matrix;
}

void TransformAnimator::makeDuplicate(TransformAnimator *target) {
    BasicTransformAnimator::makeDuplicate(target);
    target->duplicatePivotAnimatorFrom(mPivotAnimator.data());
    target->duplicateOpacityAnimatorFrom(mOpacityAnimator.data());
}

void TransformAnimator::duplicatePivotAnimatorFrom(
        QPointFAnimator *source) {
    source->makeDuplicate(mPivotAnimator.data());
}

void TransformAnimator::duplicateOpacityAnimatorFrom(
        QrealAnimator *source) {
    source->makeDuplicate(mOpacityAnimator.data());
}
