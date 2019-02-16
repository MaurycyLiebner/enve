#ifndef KEYT_H
#define KEYT_H
#include "basedkeyt.h"

template <typename T>
class KeyT : public BasedKeyT<Key, T> {
    friend class StdSelfRef;
protected:
    KeyT(const T &value, const int &relFrame,
         AnimatorT<T> * const parentAnimator = nullptr) :
        BasedKeyT<Key, T>(value, relFrame, parentAnimator) {}

    KeyT(AnimatorT<T> * const parentAnimator = nullptr) :
        BasedKeyT<Key, T>(parentAnimator) {}
};

#endif // KEYT_H
