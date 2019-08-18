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

void FixedLenAnimationRect::writeDurationRectangle(QIODevice *target) {
    target->write(rcConstChar(&mBoundToAnimation), sizeof(bool));
    target->write(rcConstChar(&mSetMaxFrameAtLeastOnce), sizeof(bool));
    target->write(rcConstChar(&mMinAnimationFrame), sizeof(int));
    target->write(rcConstChar(&mMaxAnimationFrame), sizeof(int));
    DurationRectangle::writeDurationRectangle(target);
}

void FixedLenAnimationRect::readDurationRectangle(QIODevice *target) {
    target->read(rcChar(&mBoundToAnimation), sizeof(bool));
    target->read(rcChar(&mSetMaxFrameAtLeastOnce), sizeof(bool));
    target->read(rcChar(&mMinAnimationFrame), sizeof(int));
    target->read(rcChar(&mMaxAnimationFrame), sizeof(int));
    DurationRectangle::readDurationRectangle(target);
}
