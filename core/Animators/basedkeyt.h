#ifndef BASEDKEYT_H
#define BASEDKEYT_H
#include "key.h"
template <typename T>
class AnimatorT;

template <typename B, typename T>
class BasedKeyT : public B {
public:
    bool differsFromKey(Key *key) const {
        const auto bk = static_cast<BasedKeyT<B, T>*>(key);
        return gDiffers(bk->getValue(), mValue);
    }

    const T &getValue() { return mValue; }
    void setValue(const T &value) { mValue = value; }
    void writeKey(QIODevice *target) {
        B::writeKey(target);
        gWrite(target, mValue);
    }

    void readKey(QIODevice *target) {
        B::readKey(target);
        gRead(target, mValue);
    }
protected:
    BasedKeyT(const T &value, const int &relFrame,
              AnimatorT<T> * const parentAnimator = nullptr) :
        B(relFrame, parentAnimator) {
        mValue = value;
    }

    BasedKeyT(AnimatorT<T> * const parentAnimator = nullptr) :
        B(parentAnimator) {}
private:
    T mValue;
};

#endif // BASEDKEYT_H
