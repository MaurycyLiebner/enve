#ifndef INTERPOLATEDANIMATOR_H
#define INTERPOLATEDANIMATOR_H
#include "Animators/animatort.h"

template <typename T>
class InterpolatedAnimator : public AnimatorT<T> {
public:
    T getValueAtRelFrame(const int &relFrame) const {
        if(Animator::anim_mKeys.isEmpty())
            return this->getCurrentValue();
        int prevId; int nextId;
        this->anim_getNextAndPreviousKeyIdForRelFrameF(prevId, nextId, relFrame);
        if(nextId == prevId) return this->getKeyAtId(nextId)->getValue();
        auto prevKey = this->getKeyAtId(prevId);
        auto nextKey = this->getKeyAtId(nextId);
        T val;
        qreal prevFrame = prevKey->getRelFrame();
        qreal nextFrame = nextKey->getRelFrame();
        qreal t = (relFrame - prevFrame)/(nextFrame - prevFrame);
        gInterpolate(prevKey->getValue(), nextKey->getValue(), t, val);
        return val;
    }
protected:
    InterpolatedAnimator(const QString& name) :
        AnimatorT<T>(name) {}
};

#endif // INTERPOLATEDANIMATOR_H
