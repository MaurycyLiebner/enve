#include "fixedlenanimationrect.h"
#include "Properties/property.h"
#include "GUI/durationrectsettingsdialog.h"

int FixedLenAnimationRect::getMinAnimationFrame() const {
    return mMinAnimationFrame;
}

int FixedLenAnimationRect::getMaxAnimationFrame() const {
    return mMaxAnimationFrame;
}

void FixedLenAnimationRect::openDurationSettingsDialog(QWidget *parent) {
    const int oldMinFrame = getMinFrame();
    const int oldMaxFrame = getMaxFrame();
    const int oldMinAnimationFrame = getMinAnimationFrame();
    const int oldMaxAnimationFrame = getMaxAnimationFrame();
    DurationRectSettingsDialog dialog(mType, getMinFrame(), getMaxFrame(),
                                      getMinAnimationFrame(), parent);
    if(dialog.exec()) {
        setMinFrame(dialog.getMinFrame());
        setMaxFrame(dialog.getMaxFrame());
        setFirstAnimationFrame(dialog.getFirstAnimationFrame());
    }

    if(dialog.result() == QDialog::Accepted) {
        const int newMinFrame = getMinFrame();
        const int newMaxFrame = getMaxFrame();
        const int newMinAnimationFrame = getMinAnimationFrame();
        const int newMaxAnimationFrame = getMaxAnimationFrame();
        const int minMinFrame = qMin(oldMinFrame, newMinFrame);
        const int maxMinFrame = qMax(oldMinFrame, newMinFrame);
        const int minMaxFrame = qMin(oldMaxFrame, newMaxFrame);
        const int maxMaxFrame = qMax(oldMaxFrame, newMaxFrame);
        mChildProperty->prp_afterChangedRelRange(
                    {minMinFrame, maxMinFrame});
        mChildProperty->prp_afterChangedRelRange(
                    {minMaxFrame, maxMaxFrame});

        const int minMinAnimationFrame = qMin(oldMinAnimationFrame,
                                              newMinAnimationFrame);
        const int maxMinAnimationFrame = qMax(oldMinAnimationFrame,
                                              newMinAnimationFrame);
        const int minMaxAnimationFrame = qMin(oldMaxAnimationFrame,
                                              newMaxAnimationFrame);
        const int maxMaxAnimationFrame = qMax(oldMaxAnimationFrame,
                                              newMaxAnimationFrame);
        mChildProperty->prp_afterChangedRelRange(
                    {qMin(minMinAnimationFrame, minMaxAnimationFrame),
                    qMax(maxMinAnimationFrame, maxMaxAnimationFrame)});
    }
}

void FixedLenAnimationRect::bindToAnimationFrameRange() {
    if(getMinFrame() < getMinAnimationFrame() ||
       getMinFrame() >= getMaxAnimationFrame()) {
        setMinFrame(getMinAnimationFrame());
    }
    mMinFrame.setMinPos(getMinAnimationFrame());
    if(getMaxFrame() > getMaxAnimationFrame() ||
       getMaxFrame() <= getMinAnimationFrame()) {
        setMaxFrame(getMaxAnimationFrame());
    }
    mMaxFrame.setMaxPos(getMaxAnimationFrame());
    emit rangeChanged();
}

void FixedLenAnimationRect::setBindToAnimationFrameRange() {
    mBoundToAnimation = true;
}

void FixedLenAnimationRect::setMinAnimationFrame(const int minAnimationFrame) {
    mMinAnimationFrame = minAnimationFrame;
    if(mBoundToAnimation) bindToAnimationFrameRange();
}

void FixedLenAnimationRect::setMaxAnimationFrame(const int maxAnimationFrame) {
    if(mSetMaxFrameAtLeastOnce) {
        if(getMaxFrame() == mMaxAnimationFrame) {
            setMaxFrame(maxAnimationFrame);
        }
    } else {
        mSetMaxFrameAtLeastOnce = true;
        setMaxFrame(maxAnimationFrame);
        mMinFrame.setMaxPos(maxAnimationFrame);
    }
    mMaxAnimationFrame = maxAnimationFrame;
    if(mBoundToAnimation) bindToAnimationFrameRange();
}

void FixedLenAnimationRect::changeFramePosBy(const int change) {
    mMaxAnimationFrame += change;
    mMinAnimationFrame += change;
    mMinFrame.changeFramePosByWithoutSignal(change);
    mMaxFrame.setMinPos(getMinFrame());
    mMaxFrame.changeFramePosByWithoutSignal(change);
    mMinFrame.setMaxPos(getMaxFrame());
    if(mBoundToAnimation) bindToAnimationFrameRange();
    DurationRectangleMovable::changeFramePosBy(change);
}
