#ifndef BASEDKEYT_H
#define BASEDKEYT_H
#include "key.h"
#include "../differsinterpolate.h"
#include "../basicreadwrite.h"

template <typename B, typename T>
class BasedKeyT : public B {
public:
    bool differsFromKey(Key * const key) const {
        if(key == this) return false;
        const auto bk = static_cast<BasedKeyT<B, T>*>(key);
        return gDiffers(bk->getValue(), mValue);
    }
    T &getValue() { return mValue; }
    const T &getValue() const { return mValue; }
    void setValue(const T &value) {
        mValue = value;
        if(!this->mParentAnimator) return;
        this->mParentAnimator->anim_updateAfterChangedKey(this);
    }
    void writeKey(eWriteStream& dst) {
        B::writeKey(dst);
        dst << mValue;
    }

    void readKey(eReadStream& src) {
        B::readKey(src);
        src >> mValue;
    }
protected:
    BasedKeyT(const T &value, const int relFrame,
              Animator * const parentAnimator = nullptr) :
        B(relFrame, parentAnimator), mValue(value) {}

    BasedKeyT(const int relFrame,
              Animator * const parentAnimator = nullptr) :
        B(relFrame, parentAnimator) {}

    BasedKeyT(Animator * const parentAnimator = nullptr) :
        B(parentAnimator) {}

    T mValue;
};

#endif // BASEDKEYT_H
