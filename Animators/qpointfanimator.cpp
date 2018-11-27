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

QPointF QPointFAnimator::getCurrentPointValueAtAbsFrame(const int &frame) const {
    return getCurrentPointValueAtRelFrame(prp_absFrameToRelFrame(frame));
}

QPointF QPointFAnimator::getCurrentPointValueAtRelFrame(const int &frame) const {
    return QPointF(mXAnimator->getCurrentValueAtRelFrame(frame),
                   mYAnimator->getCurrentValueAtRelFrame(frame));
}

QPointF QPointFAnimator::getCurrentEffectivePointValueAtAbsFrame(const int &frame) const {
    return getCurrentEffectivePointValueAtRelFrame(prp_absFrameToRelFrame(frame));
}

QPointF QPointFAnimator::getCurrentEffectivePointValueAtRelFrame(const int &frame) const {
    return QPointF(mXAnimator->getCurrentEffectiveValueAtRelFrame(frame),
                   mYAnimator->getCurrentEffectiveValueAtRelFrame(frame));
}

QPointF QPointFAnimator::getCurrentPointValueAtAbsFrameF(const qreal &frame) const {
    return getCurrentPointValueAtRelFrameF(prp_absFrameToRelFrameF(frame));
}

QPointF QPointFAnimator::getCurrentPointValueAtRelFrameF(const qreal &frame) const {
    return QPointF(mXAnimator->getCurrentValueAtRelFrameF(frame),
                   mYAnimator->getCurrentValueAtRelFrameF(frame));
}

QPointF QPointFAnimator::getCurrentEffectivePointValueAtAbsFrameF(const qreal &frame) const {
    return getCurrentEffectivePointValueAtRelFrameF(prp_absFrameToRelFrameF(frame));
}

QPointF QPointFAnimator::getCurrentEffectivePointValueAtRelFrameF(const qreal &frame) const {
    return QPointF(mXAnimator->getCurrentEffectiveValueAtRelFrameF(frame),
                   mYAnimator->getCurrentEffectiveValueAtRelFrameF(frame));
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

qreal QPointFAnimator::getXValueAtRelFrame(const int &relFrame) {
    return mXAnimator->qra_getValueAtRelFrame(relFrame);
}

qreal QPointFAnimator::getYValueAtRelFrame(const int &relFrame) {
    return mYAnimator->qra_getValueAtRelFrame(relFrame);
}

qreal QPointFAnimator::getEffectiveXValueAtRelFrame(const int &relFrame) {
    return mXAnimator->qra_getEffectiveValueAtRelFrame(relFrame);
}

qreal QPointFAnimator::getEffectiveYValueAtRelFrame(const int &relFrame) {
    return mYAnimator->qra_getEffectiveValueAtRelFrame(relFrame);
}

qreal QPointFAnimator::getXValueAtRelFrameF(const qreal &relFrame) {
    return mXAnimator->qra_getValueAtRelFrameF(relFrame);
}

qreal QPointFAnimator::getYValueAtRelFrameF(const qreal &relFrame) {
    return mYAnimator->qra_getValueAtRelFrameF(relFrame);
}

qreal QPointFAnimator::getEffectiveXValueAtRelFrameF(const qreal &relFrame) {
    return mXAnimator->qra_getEffectiveValueAtRelFrameF(relFrame);
}

qreal QPointFAnimator::getEffectiveYValueAtRelFrameF(const qreal &relFrame) {
    return mYAnimator->qra_getEffectiveValueAtRelFrameF(relFrame);
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
