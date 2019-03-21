#include "Animators/qpointfanimator.h"
#include "Animators/qrealanimator.h"

QPointFAnimator::QPointFAnimator(const QString &name) :
    ComplexAnimator(name) {
    mXAnimator = SPtrCreate(QrealAnimator)("x");
    mYAnimator = SPtrCreate(QrealAnimator)("y");
    ca_addChildAnimator(mXAnimator);
    ca_addChildAnimator(mYAnimator);
}

QPointF QPointFAnimator::getCurrentPointValue() const {
    return QPointF(mXAnimator->qra_getCurrentValue(),
                   mYAnimator->qra_getCurrentValue());
}

QPointF QPointFAnimator::getCurrentEffectivePointValue() const {
    return QPointF(mXAnimator->qra_getCurrentEffectiveValue(),
                   mYAnimator->qra_getCurrentEffectiveValue());
}

QPointF QPointFAnimator::getCurrentPointValueAtAbsFrameF(const qreal &frame) const {
    return getCurrentPointValueAtRelFrame(prp_absFrameToRelFrameF(frame));
}

QPointF QPointFAnimator::getCurrentPointValueAtRelFrame(const qreal &frame) const {
    return QPointF(mXAnimator->getCurrentValueAtRelFrame(frame),
                   mYAnimator->getCurrentValueAtRelFrame(frame));
}

QPointF QPointFAnimator::getCurrentEffectivePointValueAtAbsFrameF(const qreal &frame) const {
    return getCurrentEffectivePointValueAtRelFrame(prp_absFrameToRelFrameF(frame));
}

QPointF QPointFAnimator::getCurrentEffectivePointValueAtRelFrame(const qreal &frame) const {
    return QPointF(mXAnimator->getCurrentEffectiveValueAtRelFrame(frame),
                   mYAnimator->getCurrentEffectiveValueAtRelFrame(frame));
}

void QPointFAnimator::setPrefferedValueStep(const qreal &valueStep) {
    mXAnimator->setPrefferedValueStep(valueStep);
    mYAnimator->setPrefferedValueStep(valueStep);
}

bool QPointFAnimator::getBeingTransformed() {
    return mXAnimator->getBeingTransformed() ||
            mYAnimator->getBeingTransformed();
}

qreal QPointFAnimator::getXValue() {
    return mXAnimator->qra_getCurrentValue();
}

qreal QPointFAnimator::getYValue() {
    return mYAnimator->qra_getCurrentValue();
}

qreal QPointFAnimator::getEffectiveXValue() {
    return mXAnimator->qra_getCurrentEffectiveValue();
}

qreal QPointFAnimator::getEffectiveYValue() {
    return mYAnimator->qra_getCurrentEffectiveValue();
}

qreal QPointFAnimator::getXValueAtRelFrame(const qreal &relFrame) {
    return mXAnimator->qra_getValueAtRelFrame(relFrame);
}

qreal QPointFAnimator::getYValueAtRelFrame(const qreal &relFrame) {
    return mYAnimator->qra_getValueAtRelFrame(relFrame);
}

qreal QPointFAnimator::getEffectiveXValueAtRelFrame(const qreal &relFrame) {
    return mXAnimator->qra_getEffectiveValueAtRelFrame(relFrame);
}

qreal QPointFAnimator::getEffectiveYValueAtRelFrame(const qreal &relFrame) {
    return mYAnimator->qra_getEffectiveValueAtRelFrame(relFrame);
}

void QPointFAnimator::setCurrentPointValue(const QPointF &val) {
    mXAnimator->qra_setCurrentValue(val.x());
    mYAnimator->qra_setCurrentValue(val.y());
}

void QPointFAnimator::setValuesRange(const qreal &minVal,
                                     const qreal &maxVal) {
    mXAnimator->qra_setValueRange(minVal, maxVal);
    mYAnimator->qra_setValueRange(minVal, maxVal);
}

void QPointFAnimator::setCurrentPointValueWithoutCallingUpdater(
        const QPointF &val) {
    mXAnimator->qra_setCurrentValueWithoutCallingUpdater(val.x());
    mYAnimator->qra_setCurrentValueWithoutCallingUpdater(val.y());
}

void QPointFAnimator::incCurrentValuesWithoutCallingUpdater(
        const qreal &x, const qreal &y) {
    mXAnimator->qra_incCurrentValueWithoutCallingUpdater(x);
    mYAnimator->qra_incCurrentValueWithoutCallingUpdater(y);
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
