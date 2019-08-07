#ifndef KEYT_H
#define KEYT_H
#include "basedkeyt.h"

template <typename T>
class KeyT : public BasedKeyT<Key, T> {
    e_OBJECT
protected:
    KeyT(const T &value, const int relFrame,
         Animator * const parentAnimator = nullptr) :
        BasedKeyT<Key, T>(value, relFrame, parentAnimator) {}

    KeyT(Animator * const parentAnimator = nullptr) :
        BasedKeyT<Key, T>(parentAnimator) {}
};

#endif // KEYT_H
