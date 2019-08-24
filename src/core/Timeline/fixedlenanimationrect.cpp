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

#include "fixedlenanimationrect.h"
#include "Properties/property.h"
#include "GUI/durationrectsettingsdialog.h"

int FixedLenAnimationRect::getMinAnimRelFrame() const {
    return mMinAnimationFrame;
}

int FixedLenAnimationRect::getMaxAnimRelFrame() const {
    return mMaxAnimationFrame;
}

void FixedLenAnimationRect::bindToAnimationFrameRange() {
    if(getMinRelFrame() < getMinAnimRelFrame() ||
       getMinRelFrame() >= getMaxAnimRelFrame()) {
        setMinRelFrame(getMinAnimRelFrame());
    }
    mMinFrame.setClampMin(getMinAnimRelFrame());
    if(getMaxRelFrame() > getMaxAnimRelFrame() ||
       getMaxRelFrame() <= getMinAnimRelFrame()) {
        setMaxRelFrame(getMaxAnimRelFrame());
    }
    mMaxFrame.setClampMax(getMaxAnimRelFrame());
}

void FixedLenAnimationRect::setBindToAnimationFrameRange() {
    mBoundToAnimation = true;
}

void FixedLenAnimationRect::setMinAnimRelFrame(const int min) {
    mMinAnimationFrame = min;
    if(mBoundToAnimation) bindToAnimationFrameRange();
}

void FixedLenAnimationRect::setMaxAnimRelFrame(const int max) {
    if(mSetMaxFrameAtLeastOnce) {
        if(getMaxRelFrame() == mMaxAnimationFrame) {
            setMaxRelFrame(max);
        }
    } else {
        mSetMaxFrameAtLeastOnce = true;
        setMaxRelFrame(max);
        mMinFrame.setClampMax(max);
    }
    mMaxAnimationFrame = max;
    if(mBoundToAnimation) bindToAnimationFrameRange();
}

void FixedLenAnimationRect::writeDurationRectangle(eWriteStream &dst) {
    dst << mBoundToAnimation;
    dst << mSetMaxFrameAtLeastOnce;
    dst << mMinAnimationFrame;
    dst << mMaxAnimationFrame;
    DurationRectangle::writeDurationRectangle(dst);
}

void FixedLenAnimationRect::readDurationRectangle(eReadStream& src) {
    src >> mBoundToAnimation;
    src >> mSetMaxFrameAtLeastOnce;
    src >> mMinAnimationFrame;
    src >> mMaxAnimationFrame;
    DurationRectangle::readDurationRectangle(src);
}
