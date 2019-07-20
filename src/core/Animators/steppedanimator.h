#ifndef STEPPEDANIMATOR_H
#define STEPPEDANIMATOR_H
#include "animatort.h"

template <typename T>
class SteppedAnimator : public AnimatorT<T> {
protected:
    SteppedAnimator(const QString& name) :
        AnimatorT<T>(name) {}
public:
    FrameRange prp_getIdenticalRelRange(const int relFrame) const {
        if(this->anim_mKeys.isEmpty())
            return {FrameRange::EMIN, FrameRange::EMAX};
        const auto pn = this->anim_getPrevAndNextKeyId(relFrame);
        const int prevId = pn.first;
        const int nextId = pn.second;

        Key *prevKey = this->anim_getKeyAtIndex(prevId);
        Key *nextKey = this->anim_getKeyAtIndex(nextId);
        const bool adjKeys = nextId - prevId == 1;
        Key * const keyAtRelFrame = adjKeys ? nullptr :
                                              this->anim_getKeyAtIndex(pn.first + 1);
        Key *prevPrevKey = keyAtRelFrame ? keyAtRelFrame : prevKey;
        Key *prevNextKey = keyAtRelFrame ? keyAtRelFrame : prevKey;

        int fId = relFrame;
        int lId = relFrame;

        while(true) {
            if(!prevKey) {
                fId = FrameRange::EMIN;
                break;
            }
            if(prevPrevKey) {
                if(prevKey->differsFromKey(prevPrevKey)) break;
            }
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
            if(prevNextKey) {
                if(nextKey->differsFromKey(prevNextKey)) break;
            } else break;
            prevNextKey = nextKey;
            nextKey = nextKey->getNextKey();
        }

        return {fId, lId};
    }
protected:
    T getValueAtRelFrameK(const qreal frame,
                          const KeyT<T> * const prevKey,
                          const KeyT<T> * const nextKey) const {
        Q_UNUSED(frame);
        if(prevKey) return prevKey->getValue();
        return nextKey->getValue();
    }
};

#endif // STEPPEDANIMATOR_H
