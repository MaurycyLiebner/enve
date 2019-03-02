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
        if(range.inRange(this->anim_mCurrentRelFrame)) {
            this->updateValueFromCurrentFrame();
        }
        Animator::prp_updateAfterChangedRelFrameRange(range);
    }

    void anim_setAbsFrame(const int &frame) {
        Animator::anim_setAbsFrame(frame);
        if(this->anim_hasKeys()) {
            const T newVal = getValueAtRelFrame(this->anim_mCurrentRelFrame);
            if(gDiffers(newVal, mCurrentValue)) {
                mCurrentValue = newVal;
                this->anim_callFrameChangeUpdater();
            }
        }
    }

    T getValueAtAbsFrame(const qreal &frame) const {
        return getValueAtRelFrame(this->prp_absFrameToRelFrameF(frame));
    }

    T getValueAtRelFrame(const qreal &frame) const {
        if(this->anim_mKeys.isEmpty()) {
            return this->getCurrentValue();
        }
        int prevId;
        int nextId;
        if(this->anim_getNextAndPreviousKeyIdForRelFrameF(prevId, nextId,
                                                          frame)) {
            if(nextId == prevId) {
                return getKeyAtId(nextId)->getValue();
            } else {
                const K * const prevKey = getKeyAtId(prevId);
                const K * const nextKey = getKeyAtId(nextId);
                return getValueAtRelFrameK(frame, prevKey, nextKey);
            }
        }
        return mCurrentValue;
    }

    void setCurrentValue(const T &value) {
        mCurrentValue = value;
        if(this->anim_isRecording()) {
            anim_saveCurrentValueAsKey();
        } else {
            this->prp_updateInfluenceRangeAfterChanged();
        }
    }

    const T& getCurrentValue() const {
        return mCurrentValue;
    }

    K* getKeyAtId(const int& id) const {
        return GetAsPtrTemplated(this->anim_mKeys.at(id), K);
    }

    void anim_saveCurrentValueAsKey() {
        if(!this->anim_mIsRecording) this->anim_setRecording(true);

        if(this->anim_mKeyOnCurrentFrame) {
            static_cast<K*>(this->anim_mKeyOnCurrentFrame.data())->
                    setValue(mCurrentValue);
        } else {
            auto newKey = SPtrCreateTemplated(K)(
                        mCurrentValue, this->anim_mCurrentRelFrame, this);
            this->anim_appendKey(GetAsSPtr(newKey, Key));
            this->anim_mKeyOnCurrentFrame = newKey.get();
        }
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
    }

protected:
    BasedAnimatorT(const QString& name) : B(name) {}

    virtual T getValueAtRelFrameK(const qreal &frame,
                                  const K * const prevKey,
                                  const K * const nextKey) const = 0;

    void updateValueFromCurrentFrame() {
        mCurrentValue = getValueAtAbsFrame(this->anim_mCurrentAbsFrame);
    }

    T mCurrentValue;
};

#endif // BASEDANIMATORT_H
