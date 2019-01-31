#ifndef STEPPEDANIMATOR_H
#define STEPPEDANIMATOR_H
#include "Animators/animatort.h"

template <typename T>
class SteppedAnimator : public AnimatorT<T> {
public:
    T getValueAtRelFrame(const int &relFrame) const {
        if(this->anim_mKeys.isEmpty()) {
            return this->getCurrentValue();
        }
        Key *key;
        if(this->prp_isKeyOnCurrentFrame()) {
            key = this->anim_mKeyOnCurrentFrame;
        } else {
            key = this->anim_getPrevKey(relFrame);
        }
        if(!key) {
            key = this->anim_getNextKey(relFrame);
        }
        return static_cast<KeyT<T>*>(key)->getValue();
    }

    FrameRange prp_getIdenticalRelFrameRange(const int &relFrame) const {
        if(this->anim_mKeys.isEmpty()) return {INT_MIN, INT_MAX};
        int prevId;
        int nextId;
        this->anim_getNextAndPreviousKeyIdForRelFrame(prevId, nextId, relFrame);

        Key *prevKey = this->anim_mKeys.at(prevId).get();
        Key *nextKey = this->anim_mKeys.at(nextId).get();
        Key *prevPrevKey = nextKey;
        Key *prevNextKey = prevKey;

        int fId = relFrame;
        int lId = relFrame;

        while(true) {
            fId = prevKey->getRelFrame();
            prevPrevKey = prevKey;
            prevKey = prevKey->getPrevKey();
            if(!prevKey) {
                fId = INT_MIN;
                break;
            }
            if(prevKey->differsFromKey(prevPrevKey)) break;
        }

        while(true) {
            lId = nextKey->getRelFrame();
            if(nextKey->differsFromKey(prevNextKey)) break;
            prevNextKey = nextKey;
            nextKey = nextKey->getNextKey();
            if(!nextKey) {
                lId = INT_MAX;
                break;
            }
        }

        return {fId, lId};
    }
protected:
    SteppedAnimator(const QString& name) :
        AnimatorT<T>(name) {}
};

#endif // STEPPEDANIMATOR_H
