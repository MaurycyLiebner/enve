#ifndef ANIMATORT_H
#define ANIMATORT_H
#include "animator.h"
#include "key.h"
#include "basicreadwrite.h"
#include "differsinterpolate.h"
#include "keyt.h"
#include "basedanimatort.h"

template <typename T>
class AnimatorT : public BasedAnimatorT<Animator, KeyT<T>, T> {
public:
protected:
    AnimatorT(const QString& name) :
        BasedAnimatorT<Animator, KeyT<T>, T>(name) {}
};

#endif // ANIMATORT_H
