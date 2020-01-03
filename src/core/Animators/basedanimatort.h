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

#ifndef BASEDANIMATORT_H
#define BASEDANIMATORT_H
#include "animator.h"
#include "../ReadWrite/basicreadwrite.h"
#include "key.h"

template <typename B, typename K, typename T>
class BasedAnimatorT : public B {
    static_assert(std::is_base_of<Animator, B>::value,
                  "BasedAnimatorT can only be used with Animator derived classes");
protected:
    BasedAnimatorT(const QString &name) : B(name) {}

    virtual T getValueAtRelFrameK(const qreal frame,
                                  const K * const prevKey,
                                  const K * const nextKey) const = 0;

    virtual void afterValueChanged() {}
public:
    class Action {
        enum Type { START, SET, FINISH };
        Action(const T& value, const Type type) :
            mValue(value), mType(type) {}
    public:
        void apply(BasedAnimatorT* const target) const {
            if(mType == START) target->prp_startTransform();
            else if(mType == SET) target->setCurrentValue(mValue);
            else if(mType == FINISH) target->prp_finishTransform();
        }

        static Action sMakeStart()
        { return Action{T(), START}; }
        static Action sMakeSet(const T& value)
        { return Action{value, SET}; }
        static Action sMakeFinish()
        { return Action{T(), FINISH}; }
    private:
        T mValue;
        Type mType;
    };

    void prp_startTransform() override;
    void prp_finishTransform() override;
    void prp_afterChangedAbsRange(const FrameRange& range,
                                  const bool clip) override;

    void anim_setAbsFrame(const int frame) override;

    void anim_addKeyAtRelFrame(const int relFrame) override;

    void prp_writeProperty(eWriteStream& dst) const override;
    void prp_readProperty(eReadStream& src) override;

    stdsptr<Key> anim_createKey() override;

    T getValueAtAbsFrame(const qreal frame) const;
    T getValueAtRelFrame(const qreal frame) const;
    void setCurrentValue(const T &value);
    const T& getCurrentValue() const
    { return mCurrentValue; }
protected:
    void updateValueFromCurrentFrame();

    T mCurrentValue;
private:
    T mSavedCurrentValue;
    bool mTransformed = false;
};

template<typename B, typename K, typename T>
void BasedAnimatorT<B, K, T>::prp_startTransform() {
    if(mTransformed) return;
    if(this->anim_isRecording() &&
            !this->anim_getKeyOnCurrentFrame()) {
        this->anim_saveCurrentValueAsKey();
    }
    mSavedCurrentValue = mCurrentValue;
    mTransformed = true;
}

template<typename B, typename K, typename T>
void BasedAnimatorT<B, K, T>::prp_finishTransform() {
    if(mTransformed) {
//        addUndoRedo(new ChangeQrealAnimatorValue(mSavedCurrentValue,
//                                                 mCurrentValue,
//                                                 this) );

        mTransformed = false;

        emit prp_finishTransform();
    }
}

template<typename B, typename K, typename T>
void BasedAnimatorT<B, K, T>::prp_afterChangedAbsRange(const FrameRange &range,
                                                       const bool clip) {
    if(range.inRange(this->anim_getCurrentAbsFrame())) {
        this->updateValueFromCurrentFrame();
    }
    B::prp_afterChangedAbsRange(range, clip);
}

template<typename B, typename K, typename T>
void BasedAnimatorT<B, K, T>::anim_setAbsFrame(const int frame) {
    B::anim_setAbsFrame(frame);
    if(this->anim_hasKeys()) {
        const T newVal = getValueAtRelFrame(this->anim_getCurrentRelFrame());
        if(gDiffers(newVal, mCurrentValue)) {
            mCurrentValue = newVal;
            this->prp_afterChangedCurrent(UpdateReason::frameChange);
        }
        afterValueChanged();
    }
}

template<typename B, typename K, typename T>
void BasedAnimatorT<B, K, T>::anim_addKeyAtRelFrame(const int relFrame) {
    if(this->anim_getKeyAtRelFrame(relFrame)) return;
    const T value = getValueAtRelFrame(relFrame);
    const auto newKey = enve::make_shared<K>(value, relFrame, this);
    this->anim_appendKey(newKey);
}

template<typename B, typename K, typename T>
void BasedAnimatorT<B, K, T>::prp_writeProperty(eWriteStream &dst) const {
    this->anim_writeKeys(dst);
    dst << mCurrentValue;
}

template<typename B, typename K, typename T>
void BasedAnimatorT<B, K, T>::prp_readProperty(eReadStream &src) {
    this->anim_readKeys(src);
    src >> mCurrentValue;
    afterValueChanged();
}

template<typename B, typename K, typename T>
stdsptr<Key> BasedAnimatorT<B, K, T>::anim_createKey() {
    return enve::make_shared<K>(this);
}

template<typename B, typename K, typename T>
T BasedAnimatorT<B, K, T>::getValueAtAbsFrame(const qreal frame) const {
    return getValueAtRelFrame(this->prp_absFrameToRelFrameF(frame));
}

template<typename B, typename K, typename T>
T BasedAnimatorT<B, K, T>::getValueAtRelFrame(const qreal frame) const {
    if(!this->anim_hasKeys()) return this->getCurrentValue();
    const auto pn = this->anim_getPrevAndNextKeyIdF(frame);
    const int prevId = pn.first;
    const int nextId = pn.second;

    const bool adjKeys = nextId - prevId == 1;
    const auto keyAtRelFrame = adjKeys ?
                nullptr :
                this->template anim_getKeyAtIndex<K>(prevId + 1);
    if(keyAtRelFrame) return keyAtRelFrame->getValue();
    const auto prevKey = this->template anim_getKeyAtIndex<K>(prevId);
    const auto nextKey = this->template anim_getKeyAtIndex<K>(nextId);

    if(prevKey && nextKey) {
        return getValueAtRelFrameK(frame, prevKey, nextKey);
    } else if(prevKey) {
        return prevKey->getValue();
    } else if(nextKey) {
        return nextKey->getValue();
    }
    return mCurrentValue;
}

template<typename B, typename K, typename T>
void BasedAnimatorT<B, K, T>::setCurrentValue(const T &value) {
    mCurrentValue = value;
    const auto currKey = this->template anim_getKeyOnCurrentFrame<K>();
    if(currKey) currKey->setValue(mCurrentValue);
    else this->prp_afterWholeInfluenceRangeChanged();

    afterValueChanged();
}

template<typename B, typename K, typename T>
void BasedAnimatorT<B, K, T>::updateValueFromCurrentFrame() {
    mCurrentValue = getValueAtAbsFrame(this->anim_getCurrentAbsFrame());
    afterValueChanged();
}

#endif // BASEDANIMATORT_H
