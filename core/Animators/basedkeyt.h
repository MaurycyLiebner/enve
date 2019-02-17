#ifndef BASEDKEYT_H
#define BASEDKEYT_H
#include "key.h"

template <typename B, typename T>
class BasedKeyT : public B {
public:
    bool differsFromKey(Key *key) const {
        if(key == this) return false;
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
              Animator * const parentAnimator = nullptr) :
        B(relFrame, parentAnimator) {
        mValue = value;
    }

    BasedKeyT(Animator * const parentAnimator = nullptr) :
        B(parentAnimator) {}
private:
    T mValue;
};

#endif // BASEDKEYT_H
