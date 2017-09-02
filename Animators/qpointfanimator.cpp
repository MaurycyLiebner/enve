#include "Animators/qpointfanimator.h"
#include "Animators/qrealanimator.h"

QPointFAnimator::QPointFAnimator() : ComplexAnimator() {
    mXAnimator = (new QrealAnimator())->ref<QrealAnimator>();
    mYAnimator = (new QrealAnimator())->ref<QrealAnimator>();
    mXAnimator->prp_setName("x");
    mYAnimator->prp_setName("y");
    ca_addChildAnimator(mXAnimator.data());
    ca_addChildAnimator(mYAnimator.data());
}

QPointF QPointFAnimator::getCurrentPointValue() const {
    return QPointF(mXAnimator->qra_getCurrentValue(),
                   mYAnimator->qra_getCurrentValue());
}

QPointF QPointFAnimator::getCurrentPointValueAtAbsFrame(const int &frame) const {
    return QPointF(mXAnimator->getCurrentValueAtAbsFrame(frame),
                   mYAnimator->getCurrentValueAtAbsFrame(frame));
}

QPointF QPointFAnimator::getCurrentPointValueAtRelFrame(const int &frame) const {
    return QPointF(mXAnimator->getCurrentValueAtRelFrame(frame),
                   mYAnimator->getCurrentValueAtRelFrame(frame));
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

qreal QPointFAnimator::getXValueAtRelFrame(const int &relFrame) {
    return mXAnimator->qra_getValueAtRelFrame(relFrame);
}

qreal QPointFAnimator::getYValueAtRelFrame(const int &relFrame) {
    return mYAnimator->qra_getValueAtRelFrame(relFrame);
}

void QPointFAnimator::setCurrentPointValue(const QPointF &val,
                                           const bool &saveUndoRedo,
                                           const bool &finish,
                                           const bool &callUpdater) {
    mXAnimator->qra_setCurrentValue(val.x(),
                                    saveUndoRedo,
                                    finish,
                                    callUpdater);
    mYAnimator->qra_setCurrentValue(val.y(),
                                    saveUndoRedo,
                                    finish,
                                    callUpdater);
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

void QPointFAnimator::incAllValues(const qreal &x,
                                   const qreal &y,
                                   const bool &saveUndoRedo,
                                   const bool &finish,
                                   const bool &callUpdater) {
    mXAnimator->qra_incAllValues(x, saveUndoRedo,
                                 finish, callUpdater);
    mYAnimator->qra_incAllValues(y, saveUndoRedo,
                                 finish, callUpdater);
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

QrealAnimator *QPointFAnimator::getXAnimator() {
    return mXAnimator.data();
}

QrealAnimator *QPointFAnimator::getYAnimator() {
    return mYAnimator.data();
}

void QPointFAnimator::multSavedValueToCurrentValue(const qreal &sx,
                                                   const qreal &sy) {
    mXAnimator->multSavedValueToCurrentValue(sx);
    mYAnimator->multSavedValueToCurrentValue(sy);
}

void QPointFAnimator::multCurrentValues(qreal sx, qreal sy) {
    mXAnimator->qra_multCurrentValue(sx);
    mYAnimator->qra_multCurrentValue(sy);
}

QPointF QPointFAnimator::getSavedPointValue() {
    return QPointF(mXAnimator->qra_getSavedValue(),
                   mYAnimator->qra_getSavedValue() );
}

qreal QPointFAnimator::getSavedXValue() {
    return mXAnimator->qra_getSavedValue();
}

qreal QPointFAnimator::getSavedYValue() {
    return mYAnimator->qra_getSavedValue();
}
