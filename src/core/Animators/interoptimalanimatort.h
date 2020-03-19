// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef INTEROPTIMALANIMATORT_H
#define INTEROPTIMALANIMATORT_H
#include "graphanimatort.h"
#include "qrealpoint.h"
#include "interpolationkeyt.h"
#include "simpletask.h"

template <typename T, typename K = InterpolationKeyT<T>>
class InterOptimalAnimatorT : public GraphAnimator {
    e_OBJECT
protected:
    InterOptimalAnimatorT(const QString& name) :
        GraphAnimator(name),
        changed([this]() { changedExec(); }) {}
public:
    void prp_startTransform() override;
    void prp_cancelTransform() override;
    void prp_finishTransform() override;

    void prp_writeProperty(eWriteStream &dst) const override;
    void prp_readProperty(eReadStream& src) override;

    void prp_afterChangedAbsRange(const FrameRange &range,
                                  const bool clip) override;

    void anim_setAbsFrame(const int frame) override;
    stdsptr<Key> anim_createKey() override;
    void anim_addKeyAtRelFrame(const int relFrame) override;

    void anim_afterKeyOnCurrentFrameChanged(Key * const key) override;

    void anim_removeAllKeys() override;

    void graph_getValueConstraints(
            GraphKey *key, const QrealPointType type,
            qreal &minValue, qreal &maxValue) const override;

    SimpleTaskScheduler changed;
    void changedExec();

    void deepCopyValue(const qreal relFrame, T &result) const;

    T * getCurrentlyEdited() const
    { return mBeingChanged; }
protected:
    T& baseValue() { return mBaseValue; }
    const T& baseValue() const { return mBaseValue; }

    bool resultUpToDate() const { return mResultUpToDate; }
    void setResultUpToDate(const bool upToDate) { mResultUpToDate = upToDate; }
private:
    void updateBaseValue();
    void deepCopyValue(const qreal relFrame,
                       K * const prevKey, K * const nextKey,
                       K * const keyAtFrame, T &result) const;
    void startBaseValueTransform();
    void finishBaseValueTransform();

    bool mChanged = false;
    bool mResultUpToDate = true;

    T mBaseValue;
    T mSavedBaseValue;
    T * mBeingChanged = &mBaseValue;
};

template <typename T, typename K>
void InterOptimalAnimatorT<T, K>::prp_afterChangedAbsRange(
        const FrameRange &range, const bool clip) {
    if(range.inRange(anim_getCurrentAbsFrame())) {
        if(mChanged) mResultUpToDate = false;
        else updateBaseValue();
    }
    GraphAnimator::prp_afterChangedAbsRange(range, clip);
}

template <typename T, typename K>
void InterOptimalAnimatorT<T, K>::startBaseValueTransform() {
    if(mChanged) return;
    mSavedBaseValue = mBaseValue;
    mChanged = true;
}

template <typename T, typename K>
void InterOptimalAnimatorT<T, K>::finishBaseValueTransform() {
    if(!mChanged) return;
    mChanged = false;
    {
        const auto oldValue = mSavedBaseValue;
        const auto newValue = mBaseValue;
        UndoRedo ur;
        ur.fUndo = [this, oldValue]() {
            mBaseValue = oldValue;
            prp_afterWholeInfluenceRangeChanged();
        };
        ur.fRedo = [this, newValue]() {
            mBaseValue = newValue;
            prp_afterWholeInfluenceRangeChanged();
        };
        prp_addUndoRedo(ur);
    }
}

template <typename T, typename K>
void InterOptimalAnimatorT<T, K>::anim_removeAllKeys() {
    if(!this->anim_hasKeys()) return;
    startBaseValueTransform();

    const T currentValue = mBaseValue;
    Animator::anim_removeAllKeys();

    mBaseValue = currentValue;
    finishBaseValueTransform();
}

template <typename T, typename K>
void InterOptimalAnimatorT<T, K>::graph_getValueConstraints(
        GraphKey *key, const QrealPointType type,
        qreal &minValue, qreal &maxValue) const {
    if(type == QrealPointType::keyPt) {
        minValue = key->getRelFrame();
        maxValue = minValue;
    } else {
        minValue = -DBL_MAX;
        maxValue = DBL_MAX;
    }
}

template <typename T, typename K>
void InterOptimalAnimatorT<T, K>::prp_startTransform() {
    if(mChanged) return;
    if(anim_isRecording() && !anim_getKeyOnCurrentFrame()) {
        anim_saveCurrentValueAsKey();
    }
    if(const auto key = anim_getKeyOnCurrentFrame<K>()) {
        mChanged = true;
        key->startValueTransform();
    } else {
        startBaseValueTransform();
    }
}

template <typename T, typename K>
void InterOptimalAnimatorT<T, K>::changedExec() {
    const auto spk = anim_getKeyOnCurrentFrame<K>();
    if(spk) anim_updateAfterChangedKey(spk);
    else prp_afterWholeInfluenceRangeChanged();
}

template <typename T, typename K>
void InterOptimalAnimatorT<T, K>::prp_cancelTransform() {
    if(!mChanged) return;
    mChanged = false;
    if(const auto key = anim_getKeyOnCurrentFrame<K>()) {
        key->cancelValueTransform();
    } else {
        mBaseValue = mSavedBaseValue;
        prp_afterWholeInfluenceRangeChanged();
    }
}

template <typename T, typename K>
void InterOptimalAnimatorT<T, K>::prp_finishTransform() {
    if(!mChanged) return;
    if(const auto key = anim_getKeyOnCurrentFrame<K>()) {
        mChanged = false;
        key->finishValueTransform();
    } else {
        finishBaseValueTransform();
    }
    updateBaseValue();
}

template <typename T, typename K>
void InterOptimalAnimatorT<T, K>::prp_readProperty(eReadStream& src) {
    anim_readKeys(src);
    mBaseValue.read(src);
    if(src.evFileVersion() > 3) {
        QString name; src >> name;
        prp_setName(name);
    }
    prp_afterWholeInfluenceRangeChanged();
}

template <typename T, typename K>
void InterOptimalAnimatorT<T, K>::prp_writeProperty(eWriteStream &dst) const {
    anim_writeKeys(dst);
    mBaseValue.write(dst);
    dst << prp_getName();
}

template <typename T, typename K>
void InterOptimalAnimatorT<T, K>::anim_setAbsFrame(const int frame) {
    if(frame == anim_getCurrentAbsFrame()) return;
    const int lastRelFrame = anim_getCurrentRelFrame();
    Animator::anim_setAbsFrame(frame);
    const bool diff = prp_differencesBetweenRelFrames(
                anim_getCurrentRelFrame(), lastRelFrame);
    if(diff) {
        updateBaseValue();
        prp_afterChangedCurrent(UpdateReason::frameChange);
    }
}

template <typename T, typename K>
void InterOptimalAnimatorT<T, K>::anim_addKeyAtRelFrame(const int relFrame) {
    if(anim_getKeyAtRelFrame(relFrame)) return;
    const auto newKey = enve::make_shared<K>(this);
    newKey->setRelFrame(relFrame);
    deepCopyValue(relFrame, newKey->getValue());
    anim_appendKeyAction(newKey);
}

template <typename T, typename K>
stdsptr<Key> InterOptimalAnimatorT<T, K>::anim_createKey() {
    return enve::make_shared<K>(this);
}

template <typename T, typename K>
void InterOptimalAnimatorT<T, K>::anim_afterKeyOnCurrentFrameChanged(Key * const key) {
    const auto spk = static_cast<K*>(key);
    if(spk) mBeingChanged = &spk->getValue();
    else mBeingChanged = &mBaseValue;
}

template <typename T, typename K>
void InterOptimalAnimatorT<T, K>::updateBaseValue() {
    const auto prevK = anim_getPrevKey<K>(anim_getCurrentRelFrame());
    const auto nextK = anim_getNextKey<K>(anim_getCurrentRelFrame());
    const auto keyAtFrame = anim_getKeyOnCurrentFrame<K>();
    mResultUpToDate = false;
    deepCopyValue(anim_getCurrentRelFrame(),
                  prevK, nextK, keyAtFrame,
                  mBaseValue);
}

template <typename T, typename K>
void InterOptimalAnimatorT<T, K>::deepCopyValue(
        const qreal relFrame, T &result) const {
    const auto prevKey = anim_getPrevKey<K>(relFrame);
    const auto nextKey = anim_getNextKey<K>(relFrame);
    const auto keyAtFrame = anim_getKeyAtRelFrame<K>(relFrame);
    deepCopyValue(relFrame, prevKey, nextKey, keyAtFrame, result);
}

template <typename T, typename K>
void InterOptimalAnimatorT<T, K>::deepCopyValue(
        const qreal relFrame,
        K * const prevKey, K * const nextKey,
        K * const keyAtFrame, T &result) const {
    if(keyAtFrame) {
        result = keyAtFrame->getValue();
    } else if(prevKey && nextKey) {
        const qreal nWeight = graph_prevKeyWeight(prevKey, nextKey, relFrame);
        const auto& prevPath = prevKey->getValue();
        const auto& nextPath = nextKey->getValue();
        gInterpolate(prevPath, nextPath, nWeight, result);
    } else if(prevKey) {
        result = prevKey->getValue();
    } else if(nextKey) {
        result = nextKey->getValue();
    } else {
        if(&result == &mBaseValue) return;
        result = mBaseValue;
    }
}

#endif // INTEROPTIMALANIMATORT_H
