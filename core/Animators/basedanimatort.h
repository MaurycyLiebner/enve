#ifndef BASEDANIMATORT_H
#define BASEDANIMATORT_H
#include "animator.h"
#include "basicreadwrite.h"
#include "key.h"

template <typename B, typename K, typename T>
class BasedAnimatorT : public B {
public:
    virtual T getValueAtRelFrame(const int &relFrame) const = 0;

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

    T mCurrentValue;
};

#endif // BASEDANIMATORT_H
