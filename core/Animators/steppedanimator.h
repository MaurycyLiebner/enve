#ifndef STEPPEDANIMATOR_H
#define STEPPEDANIMATOR_H
#include "Animators/animatort.h"

template <typename T>
class SteppedAnimator : public AnimatorT<T> {
public:
    FrameRange prp_getIdenticalRelFrameRange(const int &relFrame) const {
        if(this->anim_mKeys.isEmpty())
            return {FrameRange::EMIN, FrameRange::EMAX};
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
                fId = FrameRange::EMIN;
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
                lId = FrameRange::EMAX;
                break;
            }
        }

        return {fId, lId};
    }
protected:
    SteppedAnimator(const QString& name) :
        AnimatorT<T>(name) {}

    T getValueAtRelFrameK(const qreal &frame,
                          const KeyT<T> * const prevKey,
                          const KeyT<T> * const nextKey) const {
        Q_UNUSED(frame);
        if(prevKey) return prevKey->getValue();
        return nextKey->getValue();
    }
};

#endif // STEPPEDANIMATOR_H
