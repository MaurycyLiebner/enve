#include "Animators/qpointfanimator.h"

QPointFAnimator::QPointFAnimator() : ComplexAnimator()
{
    mXAnimator.prp_setName("x");
    mYAnimator.prp_setName("y");
    addChildAnimator(&mXAnimator);
    addChildAnimator(&mYAnimator);

    mXAnimator.blockPointer();
    mYAnimator.blockPointer();
}

#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
int QPointFAnimator::prp_saveToSql(QSqlQuery *query) {
    int xAnimatorId = mXAnimator.prp_saveToSql(query);
    int yAnimatorId = mYAnimator.prp_saveToSql(query);
    if(!query->exec(
        QString("INSERT INTO qpointfanimator (xanimatorid, yanimatorid ) "
                "VALUES (%1, %2)").
                arg(xAnimatorId).
                arg(yAnimatorId) ) ) {
        qDebug() << query->lastError() << endl << query->lastQuery();
    }

    return query->lastInsertId().toInt();
}

#include <QSqlRecord>
void QPointFAnimator::loadFromSql(int posAnimatorId) {
    QSqlQuery query;

    QString queryStr = "SELECT * FROM qpointfanimator WHERE id = " +
            QString::number(posAnimatorId);
    if(query.exec(queryStr)) {
        query.next();
        int idxanimator = query.record().indexOf("xanimatorid");
        int idyanimator = query.record().indexOf("yanimatorid");

        mXAnimator.loadFromSql(query.value(idxanimator).toInt() );
        mYAnimator.loadFromSql(query.value(idyanimator).toInt() );
    } else {
        qDebug() << "Could not load qpointfanimator with id " << posAnimatorId;
    }
}

QPointF QPointFAnimator::qra_getCurrentValue() const {
    return QPointF(mXAnimator.qra_getCurrentValue(), mYAnimator.qra_getCurrentValue());
}

QPointF QPointFAnimator::getCurrentPointValueAtFrame(const int &frame) {
    return QPointF(mXAnimator.getCurrentValueAtAbsFrame(frame),
                   mYAnimator.getCurrentValueAtAbsFrame(frame));
}

QPointF QPointFAnimator::getPointValueAtFrame(const int &frame) {
    return QPointF(mXAnimator.qra_getValueAtAbsFrame(frame),
                   mYAnimator.qra_getValueAtAbsFrame(frame));
}

void QPointFAnimator::setPrefferedValueStep(const qreal &valueStep) {
    mXAnimator.setPrefferedValueStep(valueStep);
    mYAnimator.setPrefferedValueStep(valueStep);
}

qreal QPointFAnimator::getXValue()
{
    return mXAnimator.qra_getCurrentValue();
}

qreal QPointFAnimator::getYValue()
{
    return mYAnimator.qra_getCurrentValue();
}

void QPointFAnimator::qra_setCurrentValue(QPointF val, bool finish)
{
    mXAnimator.qra_setCurrentValue(val.x(), finish);
    mYAnimator.qra_setCurrentValue(val.y(), finish);
}

void QPointFAnimator::qra_setValueRange(qreal minVal, qreal maxVal) {
    mXAnimator.qra_setValueRange(minVal, maxVal);
    mYAnimator.qra_setValueRange(minVal, maxVal);
}

void QPointFAnimator::qra_incCurrentValue(qreal x, qreal y)
{
    mXAnimator.qra_incCurrentValue(x);
    mYAnimator.qra_incCurrentValue(y);
}

void QPointFAnimator::qra_incAllValues(qreal x, qreal y) {
    mXAnimator.qra_incAllValues(x);
    mYAnimator.qra_incAllValues(y);
}

void QPointFAnimator::incSavedValueToCurrentValue(qreal incXBy, qreal incYBy) {
    mXAnimator.incSavedValueToCurrentValue(incXBy);
    mYAnimator.incSavedValueToCurrentValue(incYBy);
}

void QPointFAnimator::makeDuplicate(Animator *target) {
    QPointFAnimator *pointTarget = (QPointFAnimator*)target;

    pointTarget->duplicateXAnimatorFrom(&mXAnimator);
    pointTarget->duplicateYAnimatorFrom(&mYAnimator);
}

void QPointFAnimator::duplicateXAnimatorFrom(
        QrealAnimator *source) {
    source->makeDuplicate(&mXAnimator);
}

void QPointFAnimator::duplicateYAnimatorFrom(
        QrealAnimator *source) {
    source->makeDuplicate(&mYAnimator);
}

QrealAnimator *QPointFAnimator::getXAnimator()
{
    return &mXAnimator;
}

QrealAnimator *QPointFAnimator::getYAnimator()
{
    return &mYAnimator;
}

void QPointFAnimator::multSavedValueToCurrentValue(qreal sx, qreal sy)
{
    mXAnimator.multSavedValueToCurrentValue(sx);
    mYAnimator.multSavedValueToCurrentValue(sy);
}

void QPointFAnimator::qra_multCurrentValue(qreal sx, qreal sy)
{
    mXAnimator.qra_multCurrentValue(sx);
    mYAnimator.qra_multCurrentValue(sy);
}

QPointF QPointFAnimator::qra_getSavedValue()
{
    return QPointF(mXAnimator.qra_getSavedValue(), mYAnimator.qra_getSavedValue() );
}

qreal QPointFAnimator::getSavedXValue()
{
    return mXAnimator.qra_getSavedValue();
}

qreal QPointFAnimator::getSavedYValue()
{
    return mYAnimator.qra_getSavedValue();
}
