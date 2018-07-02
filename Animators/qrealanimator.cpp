#include "Animators/qrealanimator.h"
#include <QPainter>
#include <QDebug>
#include <QApplication>
#include "mainwindow.h"
#include "animationdockwidget.h"
#include <QMenu>
#include "qrealanimatorvalueslider.h"
#include <QWidgetAction>
#include "BoxesList/boxsinglewidget.h"
#include "qrealpoint.h"
#include "qrealkey.h"
#include "randomqrealgenerator.h"

QrealAnimator::QrealAnimator() : Animator() {

}

QrealAnimator::~QrealAnimator() {
}

void QrealAnimator::qra_setValueRange(const qreal &minVal,
                                      const qreal &maxVal) {
    mMinPossibleVal = minVal;
    mMaxPossibleVal = maxVal;
    qra_setCurrentValue(mCurrentValue);
}

void QrealAnimator::qra_incAllValues(const qreal &valInc,
                                     const bool &saveUndoRedo,
                                     const bool &finish,
                                     const bool &callUpdater) {
    Q_FOREACH(const std::shared_ptr<Key> &key, anim_mKeys) {
        ((QrealKey*)key.get())->incValue(valInc, saveUndoRedo,
                                         finish, callUpdater);
    }
    qra_incCurrentValue(valInc, saveUndoRedo,
                        finish, callUpdater);
}

QString QrealAnimator::prp_getValueText() {
    return QString::number(mCurrentValue, 'f', 2);
}

void QrealAnimator::prp_openContextMenu(const QPoint &pos) {
    QMenu menu;
    menu.addAction("Add Key");
    QAction *selected_action = menu.exec(pos);
    if(selected_action != nullptr)
    {
        if(selected_action->text() == "Add Key")
        {
            if(anim_mIsRecording) {
                anim_saveCurrentValueAsKey();
            } else {
                prp_setRecording(true);
            }
        }
    } else {

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

void QrealAnimator::prp_setRecording(bool rec) {
    if(rec) {
        anim_setRecordingWithoutChangingKeys(rec);
        anim_saveCurrentValueAsKey();
    } else {
        anim_removeAllKeys();
        qra_updateKeysPath();
        anim_setRecordingWithoutChangingKeys(rec);
    }
}

void QrealAnimator::removeThisFromGraphAnimator() {
    //mMainWindow->getKeysView()->graphRemoveViewedAnimator(this);
}

void QrealAnimator::prp_clearFromGraphView() {
    removeThisFromGraphAnimator();
}

void QrealAnimator::freezeMinMaxValues() {
    mMinMaxValuesFrozen = true;
}

qreal QrealAnimator::getCurrentValueAtAbsFrame(const int &frame) {
    return getCurrentValueAtRelFrame(prp_absFrameToRelFrame(frame));
}

qreal QrealAnimator::getCurrentValueAtRelFrame(const int &frame) const {
    if(frame == anim_mCurrentRelFrame) return mCurrentValue;
    return qra_getValueAtRelFrame(frame);
}

qreal QrealAnimator::getCurrentValueAtAbsFrameF(const qreal &frame) {
    return getCurrentValueAtRelFrame(prp_absFrameToRelFrame(frame));
}

qreal QrealAnimator::getCurrentValueAtRelFrameF(const qreal &frame) const {
    if(qAbs(frame - anim_mCurrentRelFrame) <= 0.0001) return mCurrentValue;
    return qra_getValueAtRelFrame(frame);
}

qreal QrealAnimator::getCurrentEffectiveValueAtAbsFrameF(const qreal &frame) {
    return getCurrentEffectiveValueAtRelFrame(prp_absFrameToRelFrameF(frame));
}

qreal QrealAnimator::getCurrentEffectiveValueAtAbsFrame(const int &frame) {
    return getCurrentEffectiveValueAtRelFrame(prp_absFrameToRelFrame(frame));
}

qreal QrealAnimator::getCurrentEffectiveValueAtRelFrame(const int &frame) const {
    if(mRandomGenerator.isNull()) {
        return getCurrentValueAtRelFrame(frame);
    }
    qreal val = getCurrentValueAtRelFrame(frame) +
            mRandomGenerator->getDevAtRelFrame(frame);
    return qMin(mMaxPossibleVal, qMax(mMinPossibleVal, val));
}

qreal QrealAnimator::getCurrentEffectiveValueAtRelFrameF(const qreal &frame) const {
    if(mRandomGenerator.isNull()) {
        return getCurrentValueAtRelFrameF(frame);
    }
    qreal val = getCurrentValueAtRelFrameF(frame) +
            mRandomGenerator->getDevAtRelFrameF(frame);
    return qMin(mMaxPossibleVal, qMax(mMinPossibleVal, val));
}

qreal QrealAnimator::qra_getValueAtAbsFrame(const int &frame) {
    return qra_getValueAtRelFrame(prp_absFrameToRelFrame(frame));
}

qreal QrealAnimator::qra_getEffectiveValueAtAbsFrame(const int &frame) {
    return qra_getEffectiveValueAtRelFrame(prp_absFrameToRelFrame(frame));
}

QrealKey *QrealAnimator::getQrealKeyAtId(const int &id) const {
    return (QrealKey*)anim_mKeys.at(id).get();
}
#include "fakecomplexanimator.h"
void QrealAnimator::setGenerator(RandomQrealGenerator *generator) {
    if(generator == mRandomGenerator.data()) return;
    if(generator == nullptr) {
        mFakeComplexAnimator->ca_removeChildAnimator(mRandomGenerator.data());
        disableFakeComplexAnimatrIfNotNeeded();
    } else {
        if(mRandomGenerator.isNull()) {
            enableFakeComplexAnimator();
        } else {
            mFakeComplexAnimator->ca_removeChildAnimator(mRandomGenerator.data());
        }

        mFakeComplexAnimator->ca_addChildAnimator(generator);
    }
    if(generator == nullptr) {
        mRandomGenerator.reset();
    } else {
        mRandomGenerator = generator->ref<RandomQrealGenerator>();
    }

    prp_updateInfluenceRangeAfterChanged();
}

bool QrealAnimator::qra_hasNoise() {
    return !mRandomGenerator.isNull();
}

qreal QrealAnimator::qra_getValueAtRelFrame(const int &frame) const {
    int prevId;
    int nextId;
    if(anim_getNextAndPreviousKeyIdForRelFrame(&prevId, &nextId, frame) ) {
        if(nextId == prevId) {
            return getQrealKeyAtId(nextId)->getValue();
        } else {
            QrealKey *prevKey = getQrealKeyAtId(prevId);
            QrealKey *nextKey = getQrealKeyAtId(nextId);
            return qra_getValueAtRelFrame(frame, prevKey, nextKey);
        }
    }
    return mCurrentValue;
}

qreal QrealAnimator::qra_getValueAtRelFrameF(const qreal &frame) const {
    int prevId;
    int nextId;
    if(anim_getNextAndPreviousKeyIdForRelFrameF(&prevId, &nextId, frame) ) {
        if(nextId == prevId) {
            return getQrealKeyAtId(nextId)->getValue();
        } else {
            QrealKey *prevKey = getQrealKeyAtId(prevId);
            QrealKey *nextKey = getQrealKeyAtId(nextId);
            return qra_getValueAtRelFrameF(frame, prevKey, nextKey);
        }
    }
    return mCurrentValue;
}

qreal QrealAnimator::qra_getEffectiveValueAtRelFrameF(const qreal &frame) const {
    if(mRandomGenerator.isNull()) {
        return qra_getValueAtRelFrameF(frame);
    }
    qreal val = qra_getValueAtRelFrameF(frame) +
            mRandomGenerator->getDevAtRelFrame(frame);
    return qMin(mMaxPossibleVal, qMax(mMinPossibleVal, val));
}


qreal QrealAnimator::qra_getEffectiveValueAtRelFrame(const int &frame) const {
    if(mRandomGenerator.isNull()) {
        return qra_getValueAtRelFrame(frame);
    }
    qreal val = qra_getValueAtRelFrame(frame) +
            mRandomGenerator->getDevAtRelFrame(frame);
    return qMin(mMaxPossibleVal, qMax(mMinPossibleVal, val));
}

qreal QrealAnimator::qra_getValueAtRelFrameF(const qreal &frame,
                                             QrealKey *prevKey,
                                             QrealKey *nextKey) const {
    qreal t = tFromX(prevKey->getRelFrame(),
                     prevKey->getEndValueFrame(),
                     nextKey->getStartValueFrame(),
                     nextKey->getRelFrame(), frame);
    qreal p0y = prevKey->getValue();
    qreal p1y = prevKey->getEndValue();
    qreal p2y = nextKey->getStartValue();
    qreal p3y = nextKey->getValue();
    return qclamp(calcCubicBezierVal(p0y, p1y, p2y, p3y, t),
                  mMinPossibleVal, mMaxPossibleVal);
}


qreal QrealAnimator::qra_getValueAtRelFrame(const int &frame,
                                            QrealKey *prevKey,
                                            QrealKey *nextKey) const {
    qreal t = tFromX(prevKey->getRelFrame(),
                     prevKey->getEndValueFrame(),
                     nextKey->getStartValueFrame(),
                     nextKey->getRelFrame(), frame);
    qreal p0y = prevKey->getValue();
    qreal p1y = prevKey->getEndValue();
    qreal p2y = nextKey->getStartValue();
    qreal p3y = nextKey->getValue();
    return qclamp(calcCubicBezierVal(p0y, p1y, p2y, p3y, t),
                  mMinPossibleVal, mMaxPossibleVal);
}

qreal QrealAnimator::qra_getCurrentValue() const {
    return mCurrentValue;
}

qreal QrealAnimator::qra_getCurrentEffectiveValue() {
    if(mRandomGenerator.isNull()) {
        return mCurrentValue;
    }
    qreal val = mCurrentValue +
            mRandomGenerator->getDevAtRelFrame(anim_mCurrentRelFrame);
    return qMin(mMaxPossibleVal, qMax(mMinPossibleVal, val));
}

void QrealAnimator::qra_setCurrentValue(qreal newValue,
                                        const bool &saveUndoRedo,
                                        const bool &finish,
                                        const bool &callUpdater) {
    newValue = clamp(newValue, mMinPossibleVal, mMaxPossibleVal);

    if(saveUndoRedo) {
        prp_startTransform();
        mCurrentValue = newValue;
        emit valueChangedSignal(mCurrentValue);
        prp_finishTransform();
        return;
    }

    if(newValue == mCurrentValue) return;
    mCurrentValue = newValue;
    if(prp_isKeyOnCurrentFrame()) {
        qra_saveCurrentValueToKey((QrealKey*)anim_mKeyOnCurrentFrame,
                                  finish);
    } else if(finish) {
        prp_updateInfluenceRangeAfterChanged();
    }

    emit valueChangedSignal(mCurrentValue);
    if(callUpdater) {
        prp_callUpdater();
    }

    //qra_updateKeysPath();
}

void QrealAnimator::qra_updateValueFromCurrentFrame() {
    qra_setCurrentValue(qra_getValueAtAbsFrame(anim_mCurrentAbsFrame));
}

void QrealAnimator::qra_saveCurrentValueToKey(QrealKey *key,
                                              const bool &finish) {
    qra_saveValueToKey(key, mCurrentValue, finish);
}

void QrealAnimator::qra_saveValueToKey(const int &frame,
                                       const qreal &value) {
    QrealKey *keyAtFrame = (QrealKey*)anim_getKeyAtAbsFrame(frame);
    if(keyAtFrame == nullptr) {
        keyAtFrame = new QrealKey(this);
        keyAtFrame->setRelFrame(frame);
        keyAtFrame->setValue(value);
        anim_appendKey(keyAtFrame);
        qra_updateKeysPath();
    } else {
        qra_saveValueToKey(keyAtFrame, value);
    }
}

void QrealAnimator::qra_saveValueToKey(QrealKey *key,
                                       const qreal &value,
                                       const bool &saveUndoRedo,
                                       const bool &finish) {
    key->setValue(value, saveUndoRedo, finish);

    if(anim_mIsCurrentAnimator) {
        graphScheduleUpdateAfterKeysChanged();
    }
    qra_updateKeysPath();
}

void QrealAnimator::prp_setAbsFrame(const int &frame) {
    Animator::prp_setAbsFrame(frame);
    qreal newValue = qra_getValueAtRelFrame(anim_mCurrentRelFrame);
    if(newValue == mCurrentValue) return;
    mCurrentValue = newValue;

    emit valueChangedSignal(mCurrentValue);

    anim_callFrameChangeUpdater();
}

void QrealAnimator::saveValueAtAbsFrameAsKey(const int &frame) {
    QrealKey *keyAtFrame = (QrealKey*)anim_getKeyAtAbsFrame(frame);
    if(keyAtFrame == nullptr) {
        qreal value = qra_getValueAtAbsFrame(frame);
        keyAtFrame = new QrealKey(this);
        keyAtFrame->setRelFrame(frame);
        keyAtFrame->setValue(value);
        anim_appendKey(keyAtFrame);
        qra_updateKeysPath();
    } else {
        qra_saveCurrentValueToKey(keyAtFrame);
    }
}

void QrealAnimator::anim_saveCurrentValueAsKey() {
    if(!anim_mIsRecording) {
        prp_setRecording(true);
        return;
    }

    if(anim_mKeyOnCurrentFrame == nullptr) {
        anim_mKeyOnCurrentFrame = new QrealKey(anim_mCurrentRelFrame,
                                               mCurrentValue, this);
        anim_appendKey(anim_mKeyOnCurrentFrame,
                       true,
                       false);
        qra_updateKeysPath();
    } else {
        qra_saveCurrentValueToKey((QrealKey*)anim_mKeyOnCurrentFrame);
    }
}

void QrealAnimator::anim_removeAllKeys() {
    if(anim_mKeys.isEmpty()) return;
    qreal currentValue = mCurrentValue;

    QList<std::shared_ptr<Key> > keys = anim_mKeys;
    Q_FOREACH(const std::shared_ptr<Key> &key, keys) {
        anim_removeKey(key.get());
    }
    qra_setCurrentValue(currentValue);
    anim_mKeyOnCurrentFrame = nullptr;
}

void QrealAnimator::anim_mergeKeysIfNeeded() {
    Animator::anim_mergeKeysIfNeeded();
    qra_updateKeysPath();
}

void QrealAnimator::anim_appendKey(Key *newKey,
                                   const bool &saveUndoRedo,
                                   const bool &update) {
    Animator::anim_appendKey(newKey, saveUndoRedo, update);
    //qra_updateKeysPath();
    qra_constrainCtrlsFrameValues();
    qra_updateValueFromCurrentFrame();
}

void QrealAnimator::anim_removeKey(Key *keyToRemove,
                                   const bool &saveUndoRedo) {
    Animator::anim_removeKey(keyToRemove, saveUndoRedo);
    qra_updateKeysPath();
    qra_updateValueFromCurrentFrame();
}

void QrealAnimator::anim_moveKeyToRelFrame(Key *key,
                                           const int &newFrame,
                                           const bool &saveUndoRedo,
                                           const bool &finish) {
    Animator::anim_moveKeyToRelFrame(key, newFrame, saveUndoRedo, finish);

    qra_updateKeysPath();
    qra_updateValueFromCurrentFrame();
}

void QrealAnimator::qra_updateKeysPath() {
    mKeysPath = QPainterPath();
    QrealKey *lastKey = nullptr;
    Q_FOREACH(const std::shared_ptr<Key> &key, anim_mKeys) {
        QrealKey *qaKey = ((QrealKey*)key.get());
        int keyFrame = key->getAbsFrame();
        qreal keyValue;
        if(keyFrame == anim_mCurrentAbsFrame) {
            keyValue = mCurrentValue;
        } else {
            keyValue = qaKey->getValue();
        }
        if(lastKey == nullptr) {
            mKeysPath.moveTo(-5000, keyValue);
            mKeysPath.lineTo(keyFrame, keyValue);
        } else {
            mKeysPath.cubicTo(
                        QPointF(lastKey->getEndValueFrame(),
                                lastKey->getEndValue()),
                        QPointF(qaKey->getStartValueFrame(),
                                qaKey->getStartValue()),
                        QPointF(keyFrame, keyValue));
        }
        lastKey = qaKey;
    }
    if(lastKey == nullptr) {
        mKeysPath.moveTo(-5000, mCurrentValue);
        mKeysPath.lineTo(5000, mCurrentValue);
    } else {
        mKeysPath.lineTo(5000, lastKey->getValue());
    }
}

void QrealAnimator::qra_getMinAndMaxValues(qreal *minValP,
                                           qreal *maxValP) {
    if(mMinMaxValuesFrozen) {
        *minValP = mMinPossibleVal;
        *maxValP = mMaxPossibleVal;
        return;
    }
    if(anim_mIsComplexAnimator) {
//        *minValP = mMainWindow->getKeysView()->getMinViewedFrame();
//        *maxValP = mMainWindow->getKeysView()->getMaxViewedFrame();
        return;
    }
    qreal minVal = 100000.;
    qreal maxVal = -100000.;
    if(anim_mKeys.isEmpty()) {
        *minValP = mCurrentValue - mPrefferedValueStep;
        *maxValP = mCurrentValue + mPrefferedValueStep;
    } else {
        Q_FOREACH(const std::shared_ptr<Key> &key, anim_mKeys) {
            QrealKey *qaKey = ((QrealKey*)key.get());
            qreal keyVal = qaKey->getValue();
            qreal startVal = qaKey->getStartValue();
            qreal endVal = qaKey->getEndValue();
            qreal maxKeyVal = qMax(qMax(startVal, endVal), keyVal);
            qreal minKeyVal = qMin(qMin(startVal, endVal), keyVal);
            if(maxKeyVal > maxVal) maxVal = maxKeyVal;
            if(minKeyVal < minVal) minVal = minKeyVal;
        }

        *minValP = minVal - mPrefferedValueStep;
        *maxValP = maxVal + mPrefferedValueStep;
    }
}

void QrealAnimator::qra_getMinAndMaxValuesBetweenFrames(
                    const int &startFrame, const int &endFrame,
                    qreal *minValP, qreal *maxValP) {
    qreal minVal = 100000.;
    qreal maxVal = -100000.;
    if(anim_mKeys.isEmpty()) {
        *minValP = mCurrentValue;
        *maxValP = mCurrentValue;
    } else {
        bool first = true;
        Q_FOREACH(const std::shared_ptr<Key> &key, anim_mKeys) {
            QrealKey *qaKey = ((QrealKey*)key.get());
            int keyFrame = key->getAbsFrame();
            if(keyFrame > endFrame || keyFrame < startFrame) continue;
            if(first) first = false;
            qreal keyVal = qaKey->getValue();
            qreal startVal = qaKey->getStartValue();
            qreal endVal = qaKey->getEndValue();
            qreal maxKeyVal = qMax(qMax(startVal, endVal), keyVal);
            qreal minKeyVal = qMin(qMin(startVal, endVal), keyVal);
            if(maxKeyVal > maxVal) maxVal = maxKeyVal;
            if(minKeyVal < minVal) minVal = minKeyVal;
        }
        if(first) {
            int midFrame = (startFrame + endFrame)/2;
            qreal value = qra_getValueAtAbsFrame(midFrame);
            *minValP = value;
            *maxValP = value;
        } else {
            *minValP = minVal;
            *maxValP = maxVal;
        }
    }
}

void QrealAnimator::drawKeysPath(QPainter *p,
                                 const QColor &paintColor) {
    p->save();

    QPen pen = QPen(Qt::black, 4.);
    pen.setCosmetic(true);
    p->setPen(pen);
    p->drawPath(mKeysPath);
    pen.setColor(paintColor);
    pen.setWidthF(2.);
    p->setPen(pen);
    p->drawPath(mKeysPath);

    p->setPen(Qt::NoPen);
    Q_FOREACH(const std::shared_ptr<Key> &key, anim_mKeys) {
        ((QrealKey*)key.get())->drawGraphKey(p, paintColor);
    }

    p->restore();
}

void QrealAnimator::getMinAndMaxMoveFrame(
                         QrealKey *key, QrealPoint *currentPoint,
                         qreal *minMoveFrame, qreal *maxMoveFrame) {
    if(currentPoint->isKeyPoint()) return;
    qreal keyFrame = key->getAbsFrame();

    qreal startMinMoveFrame;
    qreal endMaxMoveFrame;
    int keyId = anim_getKeyIndex(key);

    if(keyId == anim_mKeys.count() - 1) {
        endMaxMoveFrame = keyFrame + 5000.;
    } else {
        endMaxMoveFrame = anim_mKeys.at(keyId + 1)->getAbsFrame();
    }

    if(keyId == 0) {
        startMinMoveFrame = keyFrame - 5000.;
    } else {
        QrealKey *prevKey = getQrealKeyAtId(keyId - 1);
        startMinMoveFrame = prevKey->getAbsFrame();
    }

    if(key->getCtrlsMode() == CtrlsMode::CTRLS_SYMMETRIC) {
        if(currentPoint->isEndPoint()) {
            *minMoveFrame = keyFrame;
            *maxMoveFrame = qMin(endMaxMoveFrame, 2*keyFrame - startMinMoveFrame);
        } else {
            *minMoveFrame = qMax(startMinMoveFrame, 2*keyFrame - endMaxMoveFrame);
            *maxMoveFrame = keyFrame;
        }
    } else {
        if(currentPoint->isEndPoint()) {
            *minMoveFrame = keyFrame;
            *maxMoveFrame = endMaxMoveFrame;
        } else {
            *minMoveFrame = startMinMoveFrame;
            *maxMoveFrame = keyFrame;
        }
    }
}

void QrealAnimator::qra_constrainCtrlsFrameValues() {
    QrealKey *lastKey = nullptr;
    Q_FOREACH(const std::shared_ptr<Key> &key, anim_mKeys) {
        QrealKey *qaKey = ((QrealKey*)key.get());
        if(lastKey != nullptr) {
            lastKey->constrainEndCtrlMaxFrame(qaKey->getAbsFrame());
            qaKey->constrainStartCtrlMinFrame(lastKey->getAbsFrame());
        }
        lastKey = qaKey;
    }
    qra_updateKeysPath();
}

qreal QrealAnimator::qra_clampValue(const qreal &value) {
    return value;
}

qreal QrealAnimator::qra_getPrevKeyValue(QrealKey *key) {
    int keyId = anim_getKeyIndex(key);
    if(keyId == 0) return key->getValue();
    return getQrealKeyAtId(keyId - 1)->getValue();
}

qreal QrealAnimator::qra_getNextKeyValue(QrealKey *key) {
    int keyId = anim_getKeyIndex(key);
    if(keyId == anim_mKeys.count() - 1) return key->getValue();
    return getQrealKeyAtId(keyId + 1)->getValue();
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

void QrealAnimator::qra_incCurrentValue(const qreal &incBy,
                                        const bool &saveUndoRedo,
                                        const bool &finish,
                                        const bool &callUpdater) {
    qra_setCurrentValue(mCurrentValue + incBy,
                        saveUndoRedo, finish,
                        callUpdater);
}

void QrealAnimator::prp_startTransform() {
    if(mTransformed) return;
    if(anim_mIsRecording) {
        if(!prp_isKeyOnCurrentFrame()) {
            anim_saveCurrentValueAsKey();
        }
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

        if(anim_mIsCurrentAnimator) {
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

QrealPoint *QrealAnimator::qra_getPointAt(const qreal &value,
                                          const qreal &frame,
                                          const qreal &pixelsPerFrame,
                                          const qreal &pixelsPerValUnit) {
    QrealPoint *point = nullptr;
    Q_FOREACH(const std::shared_ptr<Key> &key, anim_mKeys) {
        point = ((QrealKey*)key.get())->mousePress(frame, value,
                                pixelsPerFrame, pixelsPerValUnit);
        if(point != nullptr) {
            break;
        }
    }
    return point;
}

void QrealAnimator::addKeysInRectToList(const QRectF &frameValueRect,
                                        QList<QrealKey*> *keys) {
    Q_FOREACH(const std::shared_ptr<Key> &key, anim_mKeys) {
        QrealKey *qaKey = ((QrealKey*)key.get());
        if(qaKey->isInsideRect(frameValueRect)) {
            keys->append(qaKey);
        }
    }
}
