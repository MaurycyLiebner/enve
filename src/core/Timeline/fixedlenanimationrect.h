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

#ifndef FIXEDLENANIMATIONRECT_H
#define FIXEDLENANIMATIONRECT_H
#include "animationrect.h"

class FixedLenAnimationRect : public AnimationRect {
public:
    FixedLenAnimationRect(Property& parentProp) :
        AnimationRect(parentProp) {}

    int getMinAnimRelFrame() const;
    int getMaxAnimRelFrame() const;

    void writeDurationRectangle(eWriteStream& dst);
    void readDurationRectangle(eReadStream &src);

    void setFirstAnimationFrame(const int minAnimationFrame) {
        const int animDur = mMaxAnimationFrame - mMinAnimationFrame;
        setMinAnimRelFrame(minAnimationFrame);
        setMaxAnimRelFrame(minAnimationFrame + animDur);
    }

    void bindToAnimationFrameRange();
    void setBindToAnimationFrameRange();
protected:
    void setMinAnimRelFrame(const int min);
    void setMaxAnimRelFrame(const int max);

    bool mBoundToAnimation = false;
    bool mSetMaxFrameAtLeastOnce = false;
    int mMinAnimationFrame = 0;
    int mMaxAnimationFrame = 0;
};

#endif // FIXEDLENANIMATIONRECT_H
