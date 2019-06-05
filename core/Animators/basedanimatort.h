#ifndef BASEDANIMATORT_H
#define BASEDANIMATORT_H
#include "animator.h"
#include "basicreadwrite.h"
#include "key.h"

template <typename B, typename K, typename T>
class BasedAnimatorT : public B {
    static_assert(std::is_base_of<Animator, B>::value,
                  "BasedAnimatorT can only be used with Animator derived classes");
protected:
    BasedAnimatorT(const QString& name) : B(name) {}
public:
    void prp_afterChangedAbsRange(const FrameRange& range) {
        if(range.inRange(this->anim_getCurrentAbsFrame())) {
            this->updateValueFromCurrentFrame();
        }
        B::prp_afterChangedAbsRange(range);
    }

    void anim_setAbsFrame(const int frame) {
        B::anim_setAbsFrame(frame);
        if(this->anim_hasKeys()) {
            const T newVal = getValueAtRelFrame(this->anim_getCurrentRelFrame());
            if(gDiffers(newVal, mCurrentValue)) {
                mCurrentValue = newVal;
                this->anim_callFrameChangeUpdater();
            }
            afterValueChanged();
        }
    }

    T getValueAtAbsFrame(const qreal frame) const {
        return getValueAtRelFrame(this->prp_absFrameToRelFrameF(frame));
    }

    T getValueAtRelFrame(const qreal frame) const {
        if(this->anim_mKeys.isEmpty()) return this->getCurrentValue();
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

    void setCurrentValue(const T &value) {
        mCurrentValue = value;
        if(this->anim_isRecording()) anim_saveCurrentValueAsKey();
        else this->prp_afterWholeInfluenceRangeChanged();
        afterValueChanged();
    }

    const T& getCurrentValue() const {
        return mCurrentValue;
    }

    void anim_saveCurrentValueAsKey() {
        if(this->anim_getKeyOnCurrentFrame()) {
            const auto currKey = this->template anim_getKeyOnCurrentFrame<K>();
            return currKey->setValue(mCurrentValue);
        }
        auto newKey = SPtrCreateTemplated(K)(
                    mCurrentValue, this->anim_getCurrentRelFrame(), this);
        this->anim_appendKey(newKey);
    }

    void anim_addKeyAtRelFrame(const int relFrame) {
        if(this->anim_getKeyAtRelFrame(relFrame)) return;
        const T value = getValueAtRelFrame(relFrame);
        const auto newKey = SPtrCreateTemplated(K)(value, relFrame, this);
        this->anim_appendKey(newKey);
    }

    //FrameRange prp_getIdenticalRelRange(const int relFrame) const;

    void writeProperty(QIODevice * const target) const {
        this->writeKeys(target);
        gWrite(target, mCurrentValue);
    }

    void readProperty(QIODevice *target) {
        this->readKeys(target);
        gRead(target, mCurrentValue);
        afterValueChanged();
    }

    stdsptr<Key> readKey(QIODevice *target) {
        auto newKey = SPtrCreateTemplated(K)(this);
        newKey->readKey(target);
        return std::move(newKey);
    }
protected:
    virtual T getValueAtRelFrameK(const qreal frame,
                                  const K * const prevKey,
                                  const K * const nextKey) const = 0;

    virtual void afterValueChanged() {}

    void updateValueFromCurrentFrame() {
        mCurrentValue = getValueAtAbsFrame(this->anim_getCurrentAbsFrame());
        afterValueChanged();
    }

    T mCurrentValue;
};

#endif // BASEDANIMATORT_H
