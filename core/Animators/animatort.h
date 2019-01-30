#ifndef ANIMATORT_H
#define ANIMATORT_H
#include "Animators/animator.h"
#include "Animators/key.h"
#include "basicreadwrite.h"
#include "differsinterpolate.h"
template <typename T>
class AnimatorT;

template <typename T>
class KeyT : public Key {
    friend class StdSelfRef;
public:
    bool differsFromKey(Key *key) const {
        return gDiffers(static_cast<KeyT<T>*>(key)->getValue(), mValue);
    }

    const T &getValue() { return mValue; }
    void setValue(const T &value) { mValue = value; }
    void writeKey(QIODevice *target) {
        Key::writeKey(target);
        gWrite(target, mValue);
    }
    void readKey(QIODevice *target) {
        Key::readKey(target);
        gRead(target, mValue);
    }
protected:
    KeyT(const T &value,
         const int &relFrame,
         AnimatorT<T> *parentAnimator = nullptr) :
        Key(parentAnimator) {
        mRelFrame = relFrame;
        mValue = value;
    }

    KeyT(AnimatorT<T> *parentAnimator = nullptr) :
        Key(parentAnimator) {}
private:
    T mValue;
};

template <typename T>
class AnimatorT : public Animator {
public:
    void prp_setAbsFrame(const int &frame) {
        Animator::prp_setAbsFrame(frame);
        if(prp_hasKeys()) {
            const T newVal = getValueAtRelFrame(anim_mCurrentRelFrame);
            if(gDiffers(newVal, mCurrentValue)) {
                mCurrentValue = newVal;
                anim_callFrameChangeUpdater();
            }
        }
    }

    void setCurrentValue(const T &value) {
        mCurrentValue = value;
        if(prp_isRecording()) {
            anim_saveCurrentValueAsKey();
        } else {
            prp_updateInfluenceRangeAfterChanged();
        }
    }

    const T& getCurrentValue() const {
        return mCurrentValue;
    }

    KeyT<T>* getKeyAtId(const int& id) const {
        return GetAsPtrTemplated(anim_mKeys.at(id), KeyT<T>);
    }

    void anim_saveCurrentValueAsKey() {
        if(!anim_mIsRecording) prp_setRecording(true);

        if(anim_mKeyOnCurrentFrame) {
            static_cast<KeyT<T>*>(anim_mKeyOnCurrentFrame.data())->
                    setValue(mCurrentValue);
        } else {
            auto newKey = SPtrCreateTemplated(KeyT<T>)(
                        mCurrentValue, anim_mCurrentRelFrame, this);
            anim_appendKey(GetAsSPtr(newKey, Key));
            anim_mKeyOnCurrentFrame = newKey.get();
        }
    }

    virtual T getValueAtRelFrame(const int &relFrame) const = 0;

    //FrameRange prp_getIdenticalRelFrameRange(const int &relFrame) const;

    void writeProperty(QIODevice * const target) const {
        int nKeys = anim_mKeys.count();
        target->write(rcConstChar(&nKeys), sizeof(int));
        foreach(const auto &key, anim_mKeys) {
            key->writeKey(target);
        }
        gWrite(target, mCurrentValue);
    }

    void readProperty(QIODevice *target) {
        int nKeys;
        target->read(rcChar(&nKeys), sizeof(int));
        for(int i = 0; i < nKeys; i++) {
            auto newKey = SPtrCreateTemplated(KeyT<T>)(this);
            newKey->readKey(target);
            anim_appendKey(newKey);
        }
        gRead(target, mCurrentValue);
    }

protected:
    AnimatorT(const QString& name) :
        Animator(name) {}
private:
    T mCurrentValue;
};

#endif // ANIMATORT_H
