#ifndef GRAPHKEYT_H
#define GRAPHKEYT_H
#include "basedkeyt.h"
#include "graphkey.h"

template <typename T>
class GraphKeyT : public BasedKeyT<GraphKey, T> {
    friend class StdSelfRef;
protected:
    GraphKeyT(const T &value, const int &relFrame,
         AnimatorT<T> * const parentAnimator = nullptr) :
        BasedKeyT<GraphKey, T>(value, relFrame, parentAnimator) {}

    GraphKeyT(AnimatorT<T> * const parentAnimator = nullptr) :
        BasedKeyT<GraphKey, T>(parentAnimator) {}
};

#endif // GRAPHKEYT_H
