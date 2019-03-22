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
    mCurrentBaseValue = iniVal;
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

void QrealAnimator::setValueRange(const qreal &minVal,
                                  const qreal &maxVal) {
    mMinPossibleVal = minVal;
    mMaxPossibleVal = maxVal;
    setCurrentBaseValue(mCurrentBaseValue);
}

void QrealAnimator::incAllValues(const qreal &valInc) {
    for(const auto &key : anim_mKeys) {
        GetAsPtr(key, QrealKey)->incValue(valInc);
    }
    incCurrentBaseValue(valInc);
}

QString QrealAnimator::prp_getValueText() {
    return QString::number(mCurrentBaseValue, 'f', 2);
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

void QrealAnimator::graphFixMinMaxValues() {
    mGraphMinMaxValuesFixed = true;
}

qreal QrealAnimator::getEffectiveValueAtAbsFrame(const qreal &frame) const {
    return getEffectiveValueAtRelFrame(prp_absFrameToRelFrameF(frame));
}

qreal QrealAnimator::getBaseValueAtAbsFrame(const qreal &frame) const {
    return getBaseValueAtRelFrame(prp_absFrameToRelFrameF(frame));
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

bool QrealAnimator::hasNoise() {
    return !mRandomGenerator.isNull();
}

qreal QrealAnimator::calculateBaseValueAtRelFrame(const qreal &frame) const {
    int prevId;
    int nextId;
    if(anim_getNextAndPreviousKeyIdForRelFrameF(prevId, nextId, frame)) {
        if(nextId == prevId) {
            return getQrealKeyAtId(nextId)->getValue();
        } else {
            const QrealKey * const prevKey = getQrealKeyAtId(prevId);
            const QrealKey * const nextKey = getQrealKeyAtId(nextId);

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
    }
    return mCurrentBaseValue;
}

qreal QrealAnimator::getBaseValueAtRelFrame(const qreal &frame) const {
    if(isZero4Dec(frame - anim_mCurrentRelFrame)) return mCurrentBaseValue;
    return calculateBaseValueAtRelFrame(frame);
}

qreal QrealAnimator::getEffectiveValueAtRelFrame(const qreal &frame) const {
    if(mRandomGenerator.isNull()) return getBaseValueAtRelFrame(frame);
    const qreal val = getBaseValueAtRelFrame(frame) +
            mRandomGenerator->getDevAtRelFrame(frame);
    return qMin(mMaxPossibleVal, qMax(mMinPossibleVal, val));
}

qreal QrealAnimator::getCurrentBaseValue() const {
    return mCurrentBaseValue;
}

qreal QrealAnimator::getCurrentEffectiveValue() const {
    if(mRandomGenerator.isNull()) return mCurrentBaseValue;
    const qreal val = mCurrentBaseValue +
            mRandomGenerator->getDevAtRelFrame(anim_mCurrentRelFrame);
    return qMin(mMaxPossibleVal, qMax(mMinPossibleVal, val));
}

void QrealAnimator::setCurrentBaseValue(qreal newValue) {
    newValue = clamp(newValue, mMinPossibleVal, mMaxPossibleVal);

    if(isZero4Dec(newValue - mCurrentBaseValue)) return;
    mCurrentBaseValue = newValue;
    const auto currKey = anim_getKeyOnCurrentFrame<QrealKey>();
    if(currKey) saveCurrentValueToKey(currKey);
    else prp_updateInfluenceRangeAfterChanged();

    emit valueChangedSignal(mCurrentBaseValue);

    //anim_updateKeysPath();
}

void QrealAnimator::updateBaseValueFromCurrentFrame() {
    mCurrentBaseValue = getBaseValueAtAbsFrame(anim_mCurrentAbsFrame);
    emit valueChangedSignal(mCurrentBaseValue);
}

void QrealAnimator::saveCurrentValueToKey(QrealKey * const key) {
    saveValueToKey(key, mCurrentBaseValue);
}

void QrealAnimator::saveValueToKey(const int &frame, const qreal &value) {
    const auto keyAtFrame = GetAsPtr(anim_getKeyAtAbsFrame(frame), QrealKey);
    if(!keyAtFrame) {
        const auto newKey = SPtrCreate(QrealKey)(value, frame, this);
        anim_appendKey(newKey);
        graph_updateKeysPath();
    } else {
        saveValueToKey(keyAtFrame, value);
    }
}

void QrealAnimator::saveValueToKey(QrealKey * const key, const qreal &value) {
    key->setValue(value);

    if(graph_isSelectedForGraph()) {
        graphScheduleUpdateAfterKeysChanged();
    }
    graph_updateKeysPath();
}

void QrealAnimator::anim_setAbsFrame(const int &frame) {
    Animator::anim_setAbsFrame(frame);
    const qreal newValue = calculateBaseValueAtRelFrame(anim_mCurrentRelFrame);
    if(isZero4Dec(newValue - mCurrentBaseValue)) return;
    mCurrentBaseValue = newValue;

    emit valueChangedSignal(mCurrentBaseValue);

    anim_callFrameChangeUpdater();
}

void QrealAnimator::saveValueAtAbsFrameAsKey(const int &frame) {
    const auto keyAtFrame = GetAsPtr(anim_getKeyAtAbsFrame(frame), QrealKey);
    if(!keyAtFrame) {
        const qreal value = getBaseValueAtAbsFrame(frame);
        const auto newKey = SPtrCreate(QrealKey)(value, frame, this);
        anim_appendKey(newKey);
        graph_updateKeysPath();
    } else {
        saveCurrentValueToKey(keyAtFrame);
    }
}

void QrealAnimator::anim_saveCurrentValueAsKey() {
    if(!anim_mIsRecording) {
        anim_setRecording(true);
        return;
    }

    if(!anim_getKeyOnCurrentFrame()) {
        const auto newKey = SPtrCreate(QrealKey)(mCurrentBaseValue,
                                                 anim_mCurrentRelFrame,
                                                 this);
        anim_appendKey(newKey);
        graph_updateKeysPath();
    } else {
        saveCurrentValueToKey(anim_getKeyOnCurrentFrame<QrealKey>());
    }
}

void QrealAnimator::anim_removeAllKeys() {
    if(anim_mKeys.isEmpty()) return;
    const qreal currentValue = mCurrentBaseValue;

    const auto keys = anim_mKeys;
    for(const auto& key : keys) {
        anim_removeKey(key);
    }
    setCurrentBaseValue(currentValue);
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
            keyValue = mCurrentBaseValue;
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
        graph_mKeysPath.moveTo(-5000, mCurrentBaseValue);
        graph_mKeysPath.lineTo(5000, mCurrentBaseValue);
    } else {
        graph_mKeysPath.lineTo(5000, lastKey->getValue());
    }
}

ValueRange QrealAnimator::graph_getMinAndMaxValues() const {
    if(mGraphMinMaxValuesFixed) {
        return {mMinPossibleVal, mMaxPossibleVal};
    }
    if(anim_mKeys.isEmpty()) {
        return {mCurrentBaseValue - mPrefferedValueStep,
                mCurrentBaseValue + mPrefferedValueStep};
    }
    qreal minVal = TEN_MIL;
    qreal maxVal = -TEN_MIL;
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
    if(anim_mKeys.isEmpty()) return {mCurrentBaseValue, mCurrentBaseValue};
    bool first = true;
    qreal minVal = TEN_MIL;
    qreal maxVal = -TEN_MIL;
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
        const qreal value = getBaseValueAtAbsFrame(midFrame);
        return {value, value};
    }
    return {minVal, maxVal};
}

qreal QrealAnimator::graph_clampGraphValue(const qreal &value) {
    return value;
}

void QrealAnimator::prp_retrieveSavedValue() {
    setCurrentBaseValue(mSavedCurrentValue);
}

void QrealAnimator::incSavedValueToCurrentValue(const qreal &incBy) {
    setCurrentBaseValue(mSavedCurrentValue + incBy);
}

void QrealAnimator::multSavedValueToCurrentValue(const qreal &multBy) {
    setCurrentBaseValue(mSavedCurrentValue * multBy);
}

void QrealAnimator::setCurrentBaseValueNoUpdate(
        const qreal &newValue) {
    mCurrentBaseValue = clamp(newValue, mMinPossibleVal, mMaxPossibleVal);
    emit valueChangedSignal(mCurrentBaseValue);
}

void QrealAnimator::incCurrentValueNoUpdate(
        const qreal &incBy) {
    setCurrentBaseValueNoUpdate(mCurrentBaseValue + incBy);
}

void QrealAnimator::incCurrentBaseValue(const qreal &incBy) {
    setCurrentBaseValue(mCurrentBaseValue + incBy);
}

void QrealAnimator::prp_startTransform() {
    if(mTransformed) return;
    if(anim_mIsRecording && !anim_getKeyOnCurrentFrame()) {
        anim_saveCurrentValueAsKey();
    }
    mSavedCurrentValue = mCurrentBaseValue;
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

void QrealAnimator::multCurrentBaseValue(const qreal &mult) {
    setCurrentBaseValue(mCurrentBaseValue*mult);
}

qreal QrealAnimator::getSavedBaseValue() {
    return mSavedCurrentValue;
}
