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
