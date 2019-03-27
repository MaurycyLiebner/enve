#ifndef BASEDANIMATORT_H
#define BASEDANIMATORT_H
#include "animator.h"
#include "basicreadwrite.h"
#include "key.h"

template <typename B, typename K, typename T>
class BasedAnimatorT : public B {
    static_assert(std::is_base_of<Animator, B>::value,
                  "BasedAnimatorT can only be used with Animator derived classes");
public:
    void prp_updateAfterChangedRelFrameRange(const FrameRange& range) {
        if(range.inRange(this->anim_getCurrentRelFrame())) {
            this->updateValueFromCurrentFrame();
        }
        B::prp_updateAfterChangedRelFrameRange(range);
    }

    void anim_setAbsFrame(const int &frame) {
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

    T getValueAtAbsFrame(const qreal &frame) const {
        return getValueAtRelFrame(this->prp_absFrameToRelFrameF(frame));
    }

    T getValueAtRelFrame(const qreal &frame) const {
        if(this->anim_mKeys.isEmpty()) return this->getCurrentValue();
        const auto pn = this->anim_getPrevAndNextKeyIdForRelFrameF(frame);
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
            prevKey->getValue();
        } else if(nextKey) {
            nextKey->getValue();
        }
        return mCurrentValue;
    }

    void setCurrentValue(const T &value) {
        mCurrentValue = value;
        if(this->anim_isRecording()) anim_saveCurrentValueAsKey();
        else this->prp_updateInfluenceRangeAfterChanged();
        afterValueChanged();
    }

    const T& getCurrentValue() const {
        return mCurrentValue;
    }

    void anim_saveCurrentValueAsKey() {
        if(this->anim_getKeyOnCurrentFrame()) return;
        auto newKey = SPtrCreateTemplated(K)(
                    mCurrentValue, this->anim_getCurrentRelFrame(), this);
        this->anim_appendKey(newKey);
    }

    void anim_addKeyAtRelFrame(const int& relFrame) {
        if(this->anim_getKeyAtRelFrame(relFrame)) return;
        const T value = getValueAtRelFrame(relFrame);
        const auto newKey = SPtrCreateTemplated(K)(value, relFrame, this);
        this->anim_appendKey(newKey);
    }

    //FrameRange prp_getIdenticalRelFrameRange(const int &relFrame) const;

    void writeProperty(QIODevice * const target) const {
        int nKeys = this->anim_mKeys.count();
        target->write(rcConstChar(&nKeys), sizeof(int));
        for(const auto &key : this->anim_mKeys) {
            key->writeKey(target);
        }
        gWrite(target, mCurrentValue);
    }

    void readProperty(QIODevice *target) {
        int nKeys;
        target->read(rcChar(&nKeys), sizeof(int));
        for(int i = 0; i < nKeys; i++) {
            auto newKey = SPtrCreateTemplated(K)(this);
            newKey->readKey(target);
            this->anim_appendKey(newKey);
        }
        gRead(target, mCurrentValue);
        afterValueChanged();
    }
protected:
    BasedAnimatorT(const QString& name) : B(name) {}

    virtual T getValueAtRelFrameK(const qreal &frame,
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
