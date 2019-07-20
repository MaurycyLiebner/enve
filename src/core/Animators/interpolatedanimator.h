#ifndef INTERPOLATEDANIMATOR_H
#define INTERPOLATEDANIMATOR_H
#include "animatort.h"

template <typename T>
class InterpolatedAnimator : public AnimatorT<T> {
public:
    T getValueAtRelFrame(const int relFrame) const {
        if(Animator::anim_mKeys.isEmpty())
            return this->getCurrentValue();
        int prevId; int nextId;
        this->anim_getPrevAndNextKeyIdF(prevId, nextId, relFrame);
        const auto prevKey = this->template anim_getKeyAtIndex<KeyT<T>>(prevId);
        if(nextId == prevId) return prevKey->getValue();
        const auto nextKey = this->template anim_getKeyAtIndex<KeyT<T>>(nextId);
        T val;
        const qreal prevFrame = prevKey->getRelFrame();
        const qreal nextFrame = nextKey->getRelFrame();
        const qreal t = (relFrame - prevFrame)/(nextFrame - prevFrame);
        gInterpolate(prevKey->getValue(), nextKey->getValue(), t, val);
        return val;
    }
protected:
    InterpolatedAnimator(const QString& name) :
        AnimatorT<T>(name) {}
};

#endif // INTERPOLATEDANIMATOR_H
