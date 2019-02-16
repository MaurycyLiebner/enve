#ifndef GRAPHKEYT_H
#define GRAPHKEYT_H
#include "basedkeyt.h"

template <typename T>
class GraphKeyT : public BasedKeyT<Key, T> {
    friend class StdSelfRef;
protected:
    GraphKeyT(const T &value, const int &relFrame,
         AnimatorT<T> * const parentAnimator = nullptr) :
        BasedKeyT<Key, T>(value, relFrame, parentAnimator) {}

    GraphKeyT(AnimatorT<T> * const parentAnimator = nullptr) :
        BasedKeyT<Key, T>(parentAnimator) {}
};

#endif // GRAPHKEYT_H
