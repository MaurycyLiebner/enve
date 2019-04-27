#ifndef STEPPEDANIMATOR_H
#define STEPPEDANIMATOR_H
#include "Animators/animatort.h"

template <typename T>
class SteppedAnimator : public AnimatorT<T> {
protected:
    SteppedAnimator(const QString& name) :
        AnimatorT<T>(name) {}
public:
    FrameRange prp_getIdenticalRelRange(const int &relFrame) const {
        if(this->anim_mKeys.isEmpty())
            return {FrameRange::EMIN, FrameRange::EMAX};
        const auto pn = this->anim_getPrevAndNextKeyIdForRelFrame(relFrame);
        const int prevId = pn.first;
        const int nextId = pn.second;

        Key *prevKey = this->anim_getKeyAtIndex(prevId);
        Key *nextKey = this->anim_getKeyAtIndex(nextId);
        Key *prevPrevKey = prevKey;
        Key *prevNextKey = nextKey;

        int fId = relFrame;
        int lId = relFrame;

        while(true) {
            if(!prevKey) {
                fId = FrameRange::EMIN;
                break;
            }
            if(prevKey->differsFromKey(prevPrevKey)) break;
            fId = prevKey->getRelFrame();
            prevPrevKey = prevKey;
            prevKey = prevKey->getPrevKey();
        }

        while(true) {
            if(!nextKey) {
                lId = FrameRange::EMAX;
                break;
            }
            lId = nextKey->getRelFrame() - 1;
            if(nextKey->differsFromKey(prevNextKey)) break;
            prevNextKey = nextKey;
            nextKey = nextKey->getNextKey();
        }

        return {fId, lId};
    }
protected:
    T getValueAtRelFrameK(const qreal &frame,
                          const KeyT<T> * const prevKey,
                          const KeyT<T> * const nextKey) const {
        Q_UNUSED(frame);
        if(prevKey) return prevKey->getValue();
        return nextKey->getValue();
    }
};

#endif // STEPPEDANIMATOR_H
