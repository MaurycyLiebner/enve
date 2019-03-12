#include "Animators/qrealanimator.h"
#include <QPainter>
#include <QDebug>
#include <QApplication>
#include "GUI/mainwindow.h"
#include "GUI/animationdockwidget.h"
#include <QMenu>
#include "GUI/qrealanimatorvalueslider.h"
#include <QWidgetAction>
#include "GUI/BoxesList/boxsinglewidget.h"
#include "Animators/qrealpoint.h"
#include "Animators/qrealkey.h"
#include "randomqrealgenerator.h"
#include "Animators/fakecomplexanimator.h"

QrealAnimator::QrealAnimator(const qreal &iniVal,
                             const qreal &minVal,
                             const qreal &maxVal,
                             const qreal &prefferdStep,
                             const QString &name) :
    GraphAnimator(name) {
    mCurrentValue = iniVal;
    mMinPossibleVal = minVal;
    mMaxPossibleVal = maxVal;
    mPrefferedValueStep = prefferdStep;
}

QrealAnimator::QrealAnimator(const QString &name) : GraphAnimator(name) {}

QrealAnimator::~QrealAnimator() {}

void QrealAnimator::graph_getValueConstraints(
        GraphKey *key, const QrealPointType &type,
        qreal &minMoveValue, qreal &maxMoveValue) const {
    Q_UNUSED(key);
    if(type == QrealPointType::END_POINT) {
        minMoveValue = -DBL_MAX;
        maxMoveValue = DBL_MAX;
//        auto nextKey = key->getNextKey<GraphKey>();
//        if(!nextKey) {
//            minMoveValue = mMinPossibleVal;
//            maxMoveValue = mMaxPossibleVal;
//            return;
//        }
//        qreal p0 = key->getValueForGraph();
//        qreal p2 = nextKey->getStartValue();
//        qreal p3 = nextKey->getValueForGraph();
//        int iMax = 2*(nextKey->getRelFrame() - key->getRelFrame());
//        minMoveValue = DBL_MIN;
//        maxMoveValue = DBL_MAX;
//        for(int i = 1; i < iMax; i++) {
//            qreal t = static_cast<qreal>(i)/iMax;
//            qreal maxVal = gSolveForP1(p0, p2, p3, t, mMaxPossibleVal);
//            maxMoveValue = qMin(maxMoveValue, maxVal);
//            qreal minVal = gSolveForP1(p0, p2, p3, t, mMinPossibleVal);
//            minMoveValue = qMax(minMoveValue, minVal);
//        }
    } else if(type == QrealPointType::START_POINT) {
        minMoveValue = -DBL_MAX;
        maxMoveValue = DBL_MAX;
//        auto prevKey = key->getPrevKey<GraphKey>();
//        if(!prevKey) {
//            minMoveValue = mMinPossibleVal;
//            maxMoveValue = mMaxPossibleVal;
//            return;
//        }
//        qreal p0 = prevKey->getValueForGraph();
//        qreal p1 = prevKey->getEndValue();
//        qreal p3 = key->getValueForGraph();
//        int iMax = 2*(key->getRelFrame() - prevKey->getRelFrame());
//        minMoveValue = DBL_MIN;
//        maxMoveValue = DBL_MAX;
//        for(int i = 1; i < iMax; i++) {
//            qreal t = static_cast<qreal>(i)/iMax;
//            qreal maxVal = gSolveForP2(p0, p1, p3, t, mMaxPossibleVal);
//            maxMoveValue = qMin(maxMoveValue, maxVal);
//            qreal minVal = gSolveForP2(p0, p1, p3, t, mMinPossibleVal);
//            minMoveValue = qMax(minMoveValue, minVal);
//        }
    } else { // KEY_POINT
        minMoveValue = mMinPossibleVal;
        maxMoveValue = mMaxPossibleVal;
    }
}

void QrealAnimator::qra_setValueRange(const qreal &minVal,
                                      const qreal &maxVal) {
    mMinPossibleVal = minVal;
    mMaxPossibleVal = maxVal;
    qra_setCurrentValue(mCurrentValue);
}

void QrealAnimator::qra_incAllValues(const qreal &valInc) {
    for(const auto &key : anim_mKeys) {
        GetAsPtr(key, QrealKey)->incValue(valInc);
    }
    qra_incCurrentValue(valInc);
}

QString QrealAnimator::prp_getValueText() {
    return QString::number(mCurrentValue, 'f', 2);
}

void QrealAnimator::prp_openContextMenu(const QPoint &pos) {
    QMenu menu;
    menu.addAction("Add Key");
    const QAction * const selectedAction = menu.exec(pos);
    if(selectedAction) {
        if(selectedAction->text() == "Add Key") {
            if(anim_mIsRecording) {
                anim_saveCurrentValueAsKey();
            } else {
                anim_setRecording(true);
            }
        }
    }
}

qreal QrealAnimator::getMinPossibleValue() {
    return mMinPossibleVal;
}

qreal QrealAnimator::getMaxPossibleValue() {
    return mMaxPossibleVal;
}

qreal QrealAnimator::getPrefferedValueStep() {
    return mPrefferedValueStep;
}

void QrealAnimator::setPrefferedValueStep(const qreal &valueStep) {
    mPrefferedValueStep = valueStep;
}

void QrealAnimator::anim_setRecording(const bool &rec) {
    if(rec) {
        anim_setRecordingWithoutChangingKeys(rec);
        anim_saveCurrentValueAsKey();
    } else {
        anim_removeAllKeys();
        graph_updateKeysPath();
        anim_setRecordingWithoutChangingKeys(rec);
    }
}

void QrealAnimator::graphFixMinMaxValues() {
    mGraphMinMaxValuesFixed = true;
}

qreal QrealAnimator::getCurrentValueAtAbsFrameF(const qreal &frame) {
    return getCurrentValueAtRelFrame(prp_absFrameToRelFrameF(frame));
}

qreal QrealAnimator::getCurrentValueAtRelFrame(const qreal &frame) const {
    if(isZero4Dec(frame - anim_mCurrentRelFrame)) return mCurrentValue;
    return qra_getValueAtRelFrame(frame);
}

qreal QrealAnimator::getCurrentEffectiveValueAtAbsFrameF(const qreal &frame) {
    return getCurrentEffectiveValueAtRelFrame(prp_absFrameToRelFrameF(frame));
}

qreal QrealAnimator::getCurrentEffectiveValueAtRelFrame(const qreal &frame) const {
    if(mRandomGenerator.isNull()) {
        return getCurrentValueAtRelFrame(frame);
    }
    const qreal val = getCurrentValueAtRelFrame(frame) +
            mRandomGenerator->getDevAtRelFrameF(frame);
    return qMin(mMaxPossibleVal, qMax(mMinPossibleVal, val));
}

qreal QrealAnimator::qra_getValueAtAbsFrame(const qreal &frame) const {
    return qra_getValueAtRelFrame(prp_absFrameToRelFrameF(frame));
}

QrealKey *QrealAnimator::getQrealKeyAtId(const int &id) const {
    return GetAsPtr(anim_mKeys.at(id), QrealKey);
}

void QrealAnimator::setGenerator(const qsptr<RandomQrealGenerator>& generator) {
    if(generator == mRandomGenerator.data()) return;
    if(!generator) {
        mFakeComplexAnimator->ca_removeChildAnimator(mRandomGenerator);
        disableFakeComplexAnimatrIfNotNeeded();
    } else {
        if(mRandomGenerator.isNull()) {
            enableFakeComplexAnimator();
        } else {
            mFakeComplexAnimator->ca_removeChildAnimator(mRandomGenerator);
        }

        mFakeComplexAnimator->ca_addChildAnimator(generator);
    }
    if(!generator) {
        mRandomGenerator.reset();
    } else {
        mRandomGenerator = GetAsSPtr(generator, RandomQrealGenerator);
    }

    prp_updateInfluenceRangeAfterChanged();
}

bool QrealAnimator::qra_hasNoise() {
    return !mRandomGenerator.isNull();
}

qreal QrealAnimator::qra_getValueAtRelFrame(const qreal &frame) const {
    int prevId;
    int nextId;
    if(anim_getNextAndPreviousKeyIdForRelFrameF(prevId, nextId, frame)) {
        if(nextId == prevId) {
            return getQrealKeyAtId(nextId)->getValue();
        } else {
            const QrealKey * const prevKey = getQrealKeyAtId(prevId);
            const QrealKey * const nextKey = getQrealKeyAtId(nextId);
            return qra_getValueAtRelFrame(frame, prevKey, nextKey);
        }
    }
    return mCurrentValue;
}

qreal QrealAnimator::qra_getValueAtRelFrame(const qreal &frame,
                                            const QrealKey * const prevKey,
                                            const QrealKey * const nextKey) const {
    const qCubicSegment1D seg{qreal(prevKey->getRelFrame()),
                              prevKey->getEndFrame(),
                              nextKey->getStartFrame(),
                              qreal(nextKey->getRelFrame())};
    const qreal t = gTFromX(seg, frame);
    const qreal p0y = prevKey->getValue();
    const qreal p1y = prevKey->getEndValue();
    const qreal p2y = nextKey->getStartValue();
    const qreal p3y = nextKey->getValue();
    return clamp(gCubicValueAtT({p0y, p1y, p2y, p3y}, t),
                  mMinPossibleVal, mMaxPossibleVal);
}

qreal QrealAnimator::qra_getEffectiveValueAtRelFrame(const qreal &frame) const {
    if(mRandomGenerator.isNull()) return qra_getValueAtRelFrame(frame);
    const qreal val = qra_getValueAtRelFrame(frame) +
            mRandomGenerator->getDevAtRelFrame(frame);
    return qMin(mMaxPossibleVal, qMax(mMinPossibleVal, val));
}

qreal QrealAnimator::qra_getCurrentValue() const {
    return mCurrentValue;
}

qreal QrealAnimator::qra_getCurrentEffectiveValue() {
    if(mRandomGenerator.isNull()) {
        return mCurrentValue;
    }
    const qreal val = mCurrentValue +
            mRandomGenerator->getDevAtRelFrame(anim_mCurrentRelFrame);
    return qMin(mMaxPossibleVal, qMax(mMinPossibleVal, val));
}

void QrealAnimator::qra_setCurrentValue(qreal newValue) {
    newValue = clamp(newValue, mMinPossibleVal, mMaxPossibleVal);

    if(isZero4Dec(newValue - mCurrentValue)) return;
    mCurrentValue = newValue;
    const auto currKey = anim_getKeyOnCurrentFrame<QrealKey>();
    if(currKey) {
        qra_saveCurrentValueToKey(currKey);
    } else {
        prp_updateInfluenceRangeAfterChanged();
    }

    emit valueChangedSignal(mCurrentValue);

    //anim_updateKeysPath();
}

void QrealAnimator::qra_updateValueFromCurrentFrame() {
    mCurrentValue = qra_getValueAtAbsFrame(anim_mCurrentAbsFrame);
    emit valueChangedSignal(mCurrentValue);
}

void QrealAnimator::qra_saveCurrentValueToKey(QrealKey *key) {
    qra_saveValueToKey(key, mCurrentValue);
}

void QrealAnimator::qra_saveValueToKey(const int &frame, const qreal &value) {
    const auto keyAtFrame = GetAsPtr(anim_getKeyAtAbsFrame(frame), QrealKey);
    if(!keyAtFrame) {
        auto newKey = SPtrCreate(QrealKey)(value, frame, this);
        anim_appendKey(newKey);
        graph_updateKeysPath();
    } else {
        qra_saveValueToKey(keyAtFrame, value);
    }
}

void QrealAnimator::qra_saveValueToKey(QrealKey *key, const qreal &value) {
    key->setValue(value);

    if(graph_isSelectedForGraph()) {
        graphScheduleUpdateAfterKeysChanged();
    }
    graph_updateKeysPath();
}

void QrealAnimator::anim_setAbsFrame(const int &frame) {
    Animator::anim_setAbsFrame(frame);
    const qreal newValue = qra_getValueAtRelFrame(anim_mCurrentRelFrame);
    if(isZero4Dec(newValue - mCurrentValue)) return;
    mCurrentValue = newValue;

    emit valueChangedSignal(mCurrentValue);

    anim_callFrameChangeUpdater();
}

void QrealAnimator::saveValueAtAbsFrameAsKey(const int &frame) {
    const auto keyAtFrame = GetAsPtr(anim_getKeyAtAbsFrame(frame), QrealKey);
    if(!keyAtFrame) {
        const qreal value = qra_getValueAtAbsFrame(frame);
        const auto newKey = SPtrCreate(QrealKey)(value, frame, this);
        anim_appendKey(newKey);
        graph_updateKeysPath();
    } else {
        qra_saveCurrentValueToKey(keyAtFrame);
    }
}

void QrealAnimator::anim_saveCurrentValueAsKey() {
    if(!anim_mIsRecording) {
        anim_setRecording(true);
        return;
    }

    if(!anim_getKeyOnCurrentFrame()) {
        const auto newKey = SPtrCreate(QrealKey)(mCurrentValue,
                                                 anim_mCurrentRelFrame,
                                                 this);
        anim_appendKey(newKey);
        graph_updateKeysPath();
    } else {
        qra_saveCurrentValueToKey(anim_getKeyOnCurrentFrame<QrealKey>());
    }
}

void QrealAnimator::anim_removeAllKeys() {
    if(anim_mKeys.isEmpty()) return;
    const qreal currentValue = mCurrentValue;

    const auto keys = anim_mKeys;
    for(const auto& key : keys) {
        anim_removeKey(key);
    }
    qra_setCurrentValue(currentValue);
}

void QrealAnimator::anim_mergeKeysIfNeeded() {
    Animator::anim_mergeKeysIfNeeded();
    graph_updateKeysPath();
}

void QrealAnimator::anim_appendKey(const stdsptr<Key>& newKey) {
    Animator::anim_appendKey(newKey);
    //anim_updateKeysPath();
    graph_constrainCtrlsFrameValues();
}

void QrealAnimator::anim_removeKey(const stdsptr<Key> &keyToRemove) {
    Animator::anim_removeKey(keyToRemove);
    graph_updateKeysPath();
}

void QrealAnimator::anim_moveKeyToRelFrame(Key *key, const int &newFrame) {
    Animator::anim_moveKeyToRelFrame(key, newFrame);

    graph_updateKeysPath();
}

void QrealAnimator::graph_updateKeysPath() {
    graph_mKeysPath = QPainterPath();
    QrealKey *lastKey = nullptr;
    for(const auto &key : anim_mKeys) {
        const auto qaKey = GetAsPtr(key.get(), QrealKey);
        int keyFrame = key->getAbsFrame();
        qreal keyValue;
        if(keyFrame == anim_mCurrentAbsFrame) {
            keyValue = mCurrentValue;
        } else {
            keyValue = qaKey->getValue();
        }
        if(!lastKey) {
            graph_mKeysPath.moveTo(-5000, keyValue);
            graph_mKeysPath.lineTo(keyFrame, keyValue);
        } else {
            graph_mKeysPath.cubicTo(
                        QPointF(lastKey->getEndFrame(),
                                lastKey->getEndValue()),
                        QPointF(qaKey->getStartFrame(),
                                qaKey->getStartValue()),
                        QPointF(keyFrame, keyValue));
        }
        lastKey = qaKey;
    }
    if(!lastKey) {
        graph_mKeysPath.moveTo(-5000, mCurrentValue);
        graph_mKeysPath.lineTo(5000, mCurrentValue);
    } else {
        graph_mKeysPath.lineTo(5000, lastKey->getValue());
    }
}

ValueRange QrealAnimator::graph_getMinAndMaxValues() const {
    if(mGraphMinMaxValuesFixed) {
        return {mMinPossibleVal, mMaxPossibleVal};
    }
    qreal minVal = 100000.;
    qreal maxVal = -100000.;
    if(anim_mKeys.isEmpty()) {
        return {mCurrentValue - mPrefferedValueStep,
                mCurrentValue + mPrefferedValueStep};
    }
    for(const auto &key : anim_mKeys) {
        const auto qaKey = GetAsPtr(key.get(), QrealKey);
        const qreal keyVal = qaKey->getValue();
        const qreal startVal = qaKey->getStartValue();
        const qreal endVal = qaKey->getEndValue();
        const qreal maxKeyVal = qMax(qMax(startVal, endVal), keyVal);
        const qreal minKeyVal = qMin(qMin(startVal, endVal), keyVal);
        if(maxKeyVal > maxVal) maxVal = maxKeyVal;
        if(minKeyVal < minVal) minVal = minKeyVal;
    }

    return {minVal - mPrefferedValueStep, maxVal + mPrefferedValueStep};
}

ValueRange QrealAnimator::graph_getMinAndMaxValuesBetweenFrames(
        const int &startFrame, const int &endFrame) const {
    qreal minVal = 100000.;
    qreal maxVal = -100000.;
    if(anim_mKeys.isEmpty()) return {mCurrentValue, mCurrentValue};
    bool first = true;
    for(const auto &key : anim_mKeys) {
        const auto qaKey = GetAsPtr(key.get(), QrealKey);
        const int keyFrame = key->getAbsFrame();
        if(keyFrame > endFrame || keyFrame < startFrame) continue;
        if(first) first = false;
        const qreal keyVal = qaKey->getValue();
        const qreal startVal = qaKey->getStartValue();
        const qreal endVal = qaKey->getEndValue();
        const qreal maxKeyVal = qMax(qMax(startVal, endVal), keyVal);
        const qreal minKeyVal = qMin(qMin(startVal, endVal), keyVal);
        if(maxKeyVal > maxVal) maxVal = maxKeyVal;
        if(minKeyVal < minVal) minVal = minKeyVal;
    }
    if(first) {
        const int midFrame = (startFrame + endFrame)/2;
        const qreal value = qra_getValueAtAbsFrame(midFrame);
        return {value, value};
    }
    return {minVal, maxVal};
}

qreal QrealAnimator::graph_clampGraphValue(const qreal &value) {
    return value;
}

qreal QrealAnimator::qra_getPrevKeyValue(const QrealKey * const key) {
    const int keyId = anim_getKeyIndex(key);
    if(keyId == 0) return key->getValue();
    return getQrealKeyAtId(keyId - 1)->getValue();
}

qreal QrealAnimator::qra_getNextKeyValue(const QrealKey * const key) {
    const int keyId = anim_getKeyIndex(key);
    if(keyId == anim_mKeys.count() - 1) return key->getValue();
    return getQrealKeyAtId(keyId + 1)->getValue();
}

int QrealAnimator::qra_getPrevKeyRelFrame(const QrealKey * const key) {
    const int keyId = anim_getKeyIndex(key);
    if(keyId == 0) return key->getRelFrame();
    return getQrealKeyAtId(keyId - 1)->getRelFrame();
}

int QrealAnimator::qra_getNextKeyRelFrame(const QrealKey * const key) {
    const int keyId = anim_getKeyIndex(key);
    if(keyId == anim_mKeys.count() - 1) return key->getRelFrame();
    return getQrealKeyAtId(keyId + 1)->getRelFrame();
}

void QrealAnimator::prp_retrieveSavedValue() {
    qra_setCurrentValue(mSavedCurrentValue);
}

void QrealAnimator::incSavedValueToCurrentValue(const qreal &incBy) {
    qra_setCurrentValue(mSavedCurrentValue + incBy);
}

void QrealAnimator::multSavedValueToCurrentValue(const qreal &multBy) {
    qra_setCurrentValue(mSavedCurrentValue * multBy);
}

void QrealAnimator::qra_incCurrentValue(const qreal &incBy) {
    qra_setCurrentValue(mCurrentValue + incBy);
}

void QrealAnimator::prp_startTransform() {
    if(mTransformed) return;
    if(anim_mIsRecording && !anim_getKeyOnCurrentFrame()) {
        anim_saveCurrentValueAsKey();
    }
    mSavedCurrentValue = mCurrentValue;
    mTransformed = true;
}

void QrealAnimator::prp_finishTransform() {
    if(mTransformed) {
//        addUndoRedo(new ChangeQrealAnimatorValue(mSavedCurrentValue,
//                                                 mCurrentValue,
//                                                 this) );
        if(anim_mIsRecording) {
            anim_saveCurrentValueAsKey();
        } else {
            prp_updateInfluenceRangeAfterChanged();
        }
        mTransformed = false;

        if(graph_isSelectedForGraph()) {
            graphScheduleUpdateAfterKeysChanged();
        }
        prp_callFinishUpdater();
    }
}

void QrealAnimator::prp_cancelTransform() {
    if(mTransformed) {
        mTransformed = false;
        prp_retrieveSavedValue();
        prp_callFinishUpdater();
    }
}

void QrealAnimator::qra_multCurrentValue(const qreal &mult) {
    qra_setCurrentValue(mCurrentValue*mult);
}

qreal QrealAnimator::qra_getSavedValue() {
    return mSavedCurrentValue;
}
