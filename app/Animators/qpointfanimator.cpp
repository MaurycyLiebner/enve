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
    return QPointF(mXAnimator->getCurrentBaseValue(),
                   mYAnimator->getCurrentBaseValue());
}

QPointF QPointFAnimator::getCurrentEffectivePointValue() const {
    return QPointF(mXAnimator->getCurrentEffectiveValue(),
                   mYAnimator->getCurrentEffectiveValue());
}

QPointF QPointFAnimator::getCurrentPointValueAtAbsFrameF(const qreal &frame) const {
    return getCurrentPointValueAtRelFrame(prp_absFrameToRelFrameF(frame));
}

QPointF QPointFAnimator::getCurrentPointValueAtRelFrame(const qreal &frame) const {
    return QPointF(mXAnimator->getBaseValueAtRelFrame(frame),
                   mYAnimator->getBaseValueAtRelFrame(frame));
}

QPointF QPointFAnimator::getCurrentEffectivePointValueAtAbsFrameF(const qreal &frame) const {
    return getCurrentEffectivePointValueAtRelFrame(prp_absFrameToRelFrameF(frame));
}

QPointF QPointFAnimator::getCurrentEffectivePointValueAtRelFrame(const qreal &frame) const {
    return QPointF(mXAnimator->getEffectiveValueAtRelFrame(frame),
                   mYAnimator->getEffectiveValueAtRelFrame(frame));
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
    return mXAnimator->getCurrentBaseValue();
}

qreal QPointFAnimator::getYValue() {
    return mYAnimator->getCurrentBaseValue();
}

qreal QPointFAnimator::getEffectiveXValue() {
    return mXAnimator->getCurrentEffectiveValue();
}

qreal QPointFAnimator::getEffectiveYValue() {
    return mYAnimator->getCurrentEffectiveValue();
}

qreal QPointFAnimator::getXValueAtRelFrame(const qreal &relFrame) {
    return mXAnimator->getBaseValueAtRelFrame(relFrame);
}

qreal QPointFAnimator::getYValueAtRelFrame(const qreal &relFrame) {
    return mYAnimator->getBaseValueAtRelFrame(relFrame);
}

qreal QPointFAnimator::getEffectiveXValueAtRelFrame(const qreal &relFrame) {
    return mXAnimator->getEffectiveValueAtRelFrame(relFrame);
}

qreal QPointFAnimator::getEffectiveYValueAtRelFrame(const qreal &relFrame) {
    return mYAnimator->getEffectiveValueAtRelFrame(relFrame);
}

void QPointFAnimator::setCurrentPointValue(const QPointF &val) {
    mXAnimator->setCurrentBaseValue(val.x());
    mYAnimator->setCurrentBaseValue(val.y());
}

void QPointFAnimator::setValuesRange(const qreal &minVal,
                                     const qreal &maxVal) {
    mXAnimator->setValueRange(minVal, maxVal);
    mYAnimator->setValueRange(minVal, maxVal);
}

void QPointFAnimator::setCurrentPointValueWithoutCallingUpdater(
        const QPointF &val) {
    mXAnimator->setCurrentBaseValueNoUpdate(val.x());
    mYAnimator->setCurrentBaseValueNoUpdate(val.y());
}

void QPointFAnimator::incCurrentValuesWithoutCallingUpdater(
        const qreal &x, const qreal &y) {
    mXAnimator->incCurrentValueNoUpdate(x);
    mYAnimator->incCurrentValueNoUpdate(y);
}

void QPointFAnimator::incCurrentValues(const qreal &x,
                                       const qreal &y) {
    mXAnimator->incCurrentBaseValue(x);
    mYAnimator->incCurrentBaseValue(y);
}

void QPointFAnimator::incAllValues(const qreal &x, const qreal &y) {
    mXAnimator->incAllValues(x);
    mYAnimator->incAllValues(y);
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
    mXAnimator->multCurrentBaseValue(sx);
    mYAnimator->multCurrentBaseValue(sy);
}

QPointF QPointFAnimator::getSavedPointValue() {
    return QPointF(mXAnimator->getSavedBaseValue(),
                   mYAnimator->getSavedBaseValue() );
}

qreal QPointFAnimator::getSavedXValue() {
    return mXAnimator->getSavedBaseValue();
}

qreal QPointFAnimator::getSavedYValue() {
    return mYAnimator->getSavedBaseValue();
}
