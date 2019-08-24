// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

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

#ifndef INTERPOLATEDANIMATOR_H
#define INTERPOLATEDANIMATOR_H
#include "animatort.h"

template <typename T>
class InterpolatedAnimator : public AnimatorT<T> {
public:
    T getValueAtRelFrame(const int relFrame) const {
        if(Animator::anim_mKeys.isEmpty())
            return this->getCurrentValue();
        int prevId; int nextId;
        this->anim_getPrevAndNextKeyIdF(prevId, nextId, relFrame);
        const auto prevKey = this->template anim_getKeyAtIndex<KeyT<T>>(prevId);
        if(nextId == prevId) return prevKey->getValue();
        const auto nextKey = this->template anim_getKeyAtIndex<KeyT<T>>(nextId);
        T val;
        const qreal prevFrame = prevKey->getRelFrame();
        const qreal nextFrame = nextKey->getRelFrame();
        const qreal t = (relFrame - prevFrame)/(nextFrame - prevFrame);
        gInterpolate(prevKey->getValue(), nextKey->getValue(), t, val);
        return val;
    }
protected:
    InterpolatedAnimator(const QString& name) :
        AnimatorT<T>(name) {}
};

#endif // INTERPOLATEDANIMATOR_H
