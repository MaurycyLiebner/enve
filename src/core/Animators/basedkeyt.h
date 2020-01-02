// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef BASEDKEYT_H
#define BASEDKEYT_H
#include "key.h"
#include "../differsinterpolate.h"
#include "../ReadWrite/basicreadwrite.h"

template <typename B, typename T>
class BasedKeyT : public B {
protected:
    BasedKeyT(const T &value, const int relFrame,
              Animator * const parentAnimator = nullptr) :
        B(relFrame, parentAnimator), mValue(value) {}

    BasedKeyT(const int relFrame,
              Animator * const parentAnimator = nullptr) :
        B(relFrame, parentAnimator) {}

    BasedKeyT(Animator * const parentAnimator = nullptr) :
        B(parentAnimator) {}
public:
    bool differsFromKey(Key * const key) const override {
        if(key == this) return false;
        const auto bk = static_cast<BasedKeyT<B, T>*>(key);
        return gDiffers(bk->getValue(), mValue);
    }

    void writeKey(eWriteStream& dst) override {
        B::writeKey(dst);
        dst << mValue;
    }

    void readKey(eReadStream& src) override {
        B::readKey(src);
        src >> mValue;
    }

    T &getValue() { return mValue; }
    const T &getValue() const { return mValue; }

    void setValue(const T &value) {
        mValue = value;
        if(!this->mParentAnimator) return;
        this->mParentAnimator->anim_updateAfterChangedKey(this);
    }
private:
    T mValue;
};

#endif // BASEDKEYT_H
