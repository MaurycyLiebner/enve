#include "Animators/qpointfanimator.h"

QPointFAnimator::QPointFAnimator() : ComplexAnimator()
{
    mXAnimator->prp_setName("x");
    mYAnimator->prp_setName("y");
    ca_addChildAnimator(mXAnimator.data());
    ca_addChildAnimator(mYAnimator.data());
}

#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
int QPointFAnimator::prp_saveToSql(QSqlQuery *query, const int &parentId) {
    Q_UNUSED(parentId);
    int xAnimatorId = mXAnimator->prp_saveToSql(query);
    int yAnimatorId = mYAnimator->prp_saveToSql(query);
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
void QPointFAnimator::prp_loadFromSql(const int &posAnimatorId) {
    QSqlQuery query;

    QString queryStr = "SELECT * FROM qpointfanimator WHERE id = " +
            QString::number(posAnimatorId);
    if(query.exec(queryStr)) {
        query.next();
        int idxanimator = query.record().indexOf("xanimatorid");
        int idyanimator = query.record().indexOf("yanimatorid");

        mXAnimator->prp_loadFromSql(query.value(idxanimator).toInt() );
        mYAnimator->prp_loadFromSql(query.value(idyanimator).toInt() );
    } else {
        qDebug() << "Could not load qpointfanimator with id " << posAnimatorId;
    }
}

QPointF QPointFAnimator::getCurrentPointValue() const {
    return QPointF(mXAnimator->qra_getCurrentValue(),
                   mYAnimator->qra_getCurrentValue());
}

QPointF QPointFAnimator::getCurrentPointValueAtFrame(const int &frame) {
    return QPointF(mXAnimator->getCurrentValueAtAbsFrame(frame),
                   mYAnimator->getCurrentValueAtAbsFrame(frame));
}

QPointF QPointFAnimator::getPointValueAtFrame(const int &frame) {
    return QPointF(mXAnimator->qra_getValueAtAbsFrame(frame),
                   mYAnimator->qra_getValueAtAbsFrame(frame));
}

void QPointFAnimator::setPrefferedValueStep(const qreal &valueStep) {
    mXAnimator->setPrefferedValueStep(valueStep);
    mYAnimator->setPrefferedValueStep(valueStep);
}

qreal QPointFAnimator::getXValue() {
    return mXAnimator->qra_getCurrentValue();
}

qreal QPointFAnimator::getYValue() {
    return mYAnimator->qra_getCurrentValue();
}

void QPointFAnimator::setCurrentPointValue(const QPointF &val,
                                           const bool &finish) {
    mXAnimator->qra_setCurrentValue(val.x(), finish);
    mYAnimator->qra_setCurrentValue(val.y(), finish);
}

void QPointFAnimator::setValuesRange(const qreal &minVal,
                                     const qreal &maxVal) {
    mXAnimator->qra_setValueRange(minVal, maxVal);
    mYAnimator->qra_setValueRange(minVal, maxVal);
}

void QPointFAnimator::incCurrentValues(const qreal &x,
                                       const qreal &y) {
    mXAnimator->qra_incCurrentValue(x);
    mYAnimator->qra_incCurrentValue(y);
}

void QPointFAnimator::incAllValues(const qreal &x, const qreal &y) {
    mXAnimator->qra_incAllValues(x);
    mYAnimator->qra_incAllValues(y);
}

void QPointFAnimator::incSavedValueToCurrentValue(const qreal &incXBy,
                                                  const qreal &incYBy) {
    mXAnimator->incSavedValueToCurrentValue(incXBy);
    mYAnimator->incSavedValueToCurrentValue(incYBy);
}

void QPointFAnimator::makeDuplicate(Property *target) {
    QPointFAnimator *pointTarget = (QPointFAnimator*)target;

    pointTarget->duplicateXAnimatorFrom(mXAnimator.data());
    pointTarget->duplicateYAnimatorFrom(mYAnimator.data());
}

Property *QPointFAnimator::makeDuplicate() {
    QPointFAnimator *posAnim = new QPointFAnimator();
    makeDuplicate(posAnim);
    return posAnim;
}

void QPointFAnimator::duplicateXAnimatorFrom(
        QrealAnimator *source) {
    source->makeDuplicate(mXAnimator.data());
}

void QPointFAnimator::duplicateYAnimatorFrom(
        QrealAnimator *source) {
    source->makeDuplicate(mYAnimator.data());
}

QrealAnimator *QPointFAnimator::getXAnimator()
{
    return mXAnimator.data();
}

QrealAnimator *QPointFAnimator::getYAnimator()
{
    return mYAnimator.data();
}

void QPointFAnimator::multSavedValueToCurrentValue(const qreal &sx,
                                                   const qreal &sy) {
    mXAnimator->multSavedValueToCurrentValue(sx);
    mYAnimator->multSavedValueToCurrentValue(sy);
}

void QPointFAnimator::multCurrentValues(qreal sx, qreal sy)
{
    mXAnimator->qra_multCurrentValue(sx);
    mYAnimator->qra_multCurrentValue(sy);
}

QPointF QPointFAnimator::getSavedPointValue()
{
    return QPointF(mXAnimator->qra_getSavedValue(),
                   mYAnimator->qra_getSavedValue() );
}

qreal QPointFAnimator::getSavedXValue()
{
    return mXAnimator->qra_getSavedValue();
}

qreal QPointFAnimator::getSavedYValue()
{
    return mYAnimator->qra_getSavedValue();
}
