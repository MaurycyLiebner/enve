#include "durationrectangle.h"
#include "Boxes/rendercachehandler.h"
#include "global.h"
#include "Boxes/boundingbox.h"
#include "GUI/durationrectsettingsdialog.h"

DurationRectangleMovable::DurationRectangleMovable(const Type &type) {
    mType = type;
}

void DurationRectangleMovable::setFramePos(const int &framePos) {
    mFramePos = qMin(mMaxPos, qMax(mMinPos, framePos));
}

int DurationRectangleMovable::getFramePos() const {
    return mFramePos;
}

DurationRectangleMovable *DurationRectangleMovable::getMovableAt(
                                const int &pressX,
                                const qreal &pixelsPerFrame,
                                const int &minViewedFrame) {
    const int pressedDFrame = qRound(minViewedFrame + pressX/pixelsPerFrame);
    const int thisDFrame = mFramePos - minViewedFrame;
    if(thisDFrame == pressedDFrame) return this;
    return nullptr;
}

void DurationRectangleMovable::changeFramePosBy(const int &change) {
    changeFramePosByWithoutSignal(change);
    emit posChanged(mFramePos);
    emit posChangedBy(change);
}

void DurationRectangleMovable::changeFramePosByWithoutSignal(
                                const int &change) {
    setFramePos(mFramePos + change);
}

void DurationRectangleMovable::setHovered(const bool &hovered) {
    mHovered = hovered;
}

bool DurationRectangleMovable::isHovered() {
    return mHovered;
}

void DurationRectangleMovable::pressed(const bool &shiftPressed) {
    if(!mChildProperty) return;
    if(mChildProperty->SWT_isBoundingBox()) {
        ((BoundingBox*)mChildProperty)->selectionChangeTriggered(shiftPressed);
    }
}

bool DurationRectangleMovable::isSelected() {
    if(mChildProperty) {
        if(mChildProperty->SWT_isBoundingBox()) {
            return ((BoundingBox*)mChildProperty)->isSelected();
        }
    }
    return false;
}

void DurationRectangleMovable::setMaxPos(const int &maxPos) {
    mMaxPos = maxPos - 1;
}

void DurationRectangleMovable::setMinPos(const int &minPos) {
    mMinPos = minPos + 1;
}

DurationRectangle::DurationRectangle(Property * const childProp) :
    DurationRectangleMovable(DURATION_RECT) {
    mChildProperty = childProp;

    mMinFrame.setChildProperty(childProp);
    mMaxFrame.setChildProperty(childProp);
    setMinPos(-1000000);
    setMaxPos(1000000);
    mMinFrame.setType(MIN_FRAME);
    mMaxFrame.setType(MAX_FRAME);
    mMinFrame.setMaxPos(1000000);
    mMinFrame.setMinPos(-1000000);
    mMaxFrame.setMinPos(-1000000);
    mMaxFrame.setMaxPos(1000000);
    connect(&mMinFrame, &DurationRectangleMovable::posChanged,
            &mMaxFrame, &DurationRectangleMovable::setMinPos);
    connect(&mMaxFrame, &DurationRectangleMovable::posChanged,
            &mMinFrame, &DurationRectangleMovable::setMaxPos);

    connect(&mMinFrame, &DurationRectangleMovable::posChanged,
            this, &DurationRectangle::rangeChanged);
    connect(&mMaxFrame, &DurationRectangleMovable::posChanged,
            this, &DurationRectangle::rangeChanged);

    connect(&mMinFrame, &DurationRectangleMovable::finishedTransform,
            this, &DurationRectangle::finishedRangeChange);
    connect(&mMaxFrame, &DurationRectangleMovable::finishedTransform,
            this, &DurationRectangle::finishedRangeChange);

    connect(&mMinFrame, &DurationRectangleMovable::posChangedBy,
            this, &DurationRectangle::minFrameChangedBy);
    connect(&mMaxFrame, &DurationRectangleMovable::posChangedBy,
            this, &DurationRectangle::maxFrameChangedBy);
}

void DurationRectangle::setFramesDuration(const int &duration) {
    mMaxFrame.setFramePos(getMinFrame() + duration - 1);
}

void DurationRectangle::setMinFrame(const int &minFrame) {
    mMinFrame.setFramePos(minFrame);
}

void DurationRectangle::setMaxFrame(const int &maxFrame) {
    mMaxFrame.setFramePos(maxFrame);
}

int DurationRectangle::getFrameDuration() const {
    return mMaxFrame.getFramePos() - mMinFrame.getFramePos() + 1;
}

int DurationRectangle::getMinFrame() const {
    return mMinFrame.getFramePos();
}

int DurationRectangle::getMaxFrame() const {
    return mMaxFrame.getFramePos();
}

int DurationRectangle::getMinFrameAsRelFrame() const {
    return getMinFrame() - mFramePos;
}

int DurationRectangle::getMaxFrameAsRelFrame() const {
    return getMaxFrame() - mFramePos;
}

int DurationRectangle::getMinFrameAsAbsFrame() const {
    return mChildProperty->prp_relFrameToAbsFrame(
                getMinFrameAsRelFrame());
}

int DurationRectangle::getMaxFrameAsAbsFrame() const {
    return mChildProperty->prp_relFrameToAbsFrame(
                getMaxFrameAsRelFrame());
}

void DurationRectangle::draw(QPainter * const p,
                             const QRect& drawRect,
                             const qreal& fps,
                             const qreal &pixelsPerFrame,
                             const FrameRange &absFrameRange) {
    p->save();

    const int firstRelDrawFrame = qMax(absFrameRange.fMin, getMinFrame()) -
            absFrameRange.fMin;
    const int lastRelDrawFrame = qMin(absFrameRange.fMax, getMaxFrame()) -
            absFrameRange.fMin;
    const int drawFrameSpan = lastRelDrawFrame - firstRelDrawFrame + 1;

    const QRect durRect(qFloor(firstRelDrawFrame*pixelsPerFrame),
                        drawRect.y(),
                        qCeil(drawFrameSpan*pixelsPerFrame),
                        drawRect.height());

    const int rectStartFrame = absFrameRange.fMin - mFramePos;
    const int rectEndFrame = absFrameRange.fMax - mFramePos;
    if(mRasterCacheHandler && mSoundCacheHandler) {
        const int soundHeight = drawRect.height()/3;
        const int rasterHeight = drawRect.height() - soundHeight;
        const QRect rasterRect(0, 0, drawRect.width(), rasterHeight);
        mRasterCacheHandler->drawCacheOnTimeline(p, rasterRect,
                                                 rectStartFrame,
                                                 rectEndFrame);
        const QRect soundRect(0, rasterHeight, drawRect.width(), soundHeight);
        mSoundCacheHandler->drawCacheOnTimeline(p, soundRect,
                                                rectStartFrame,
                                                rectEndFrame, fps);
    } else if(mRasterCacheHandler) {
        mRasterCacheHandler->drawCacheOnTimeline(p, drawRect,
                                                 rectStartFrame,
                                                 rectEndFrame);
    } else if(mSoundCacheHandler) {
        mSoundCacheHandler->drawCacheOnTimeline(p, drawRect,
                                                rectStartFrame,
                                                rectEndFrame, fps);
    }

    QColor fillColor;
    if(isSelected()) fillColor = QColor(50, 50, 255, 120);
    else fillColor = QColor(0, 0, 255, 120);

    p->fillRect(durRect.adjusted(0, 1, 0, -1), fillColor);
    if(mHovered) {
        p->setPen(QPen(Qt::white, .5));
        p->drawRect(durRect);
    }

    if(mMinFrame.isHovered()) p->setPen(QPen(Qt::white));
    else p->setPen(QPen(Qt::black));
    p->drawLine(durRect.topLeft(), durRect.bottomLeft());
    if(mMaxFrame.isHovered()) p->setPen(QPen(Qt::white));
    else p->setPen(QPen(Qt::black));
    p->drawLine(durRect.topRight(), durRect.bottomRight());

//    p->setPen(Qt::black);
//    p->setBrush(Qt::NoBrush);
    //p->drawRect(drawRect);
    p->restore();
}

DurationRectangleMovable *DurationRectangle::getMovableAt(
                                          const int &pressX,
                                          const qreal &pixelsPerFrame,
                                          const int &minViewedFrame) {
    const qreal startX = (getMinFrame() - minViewedFrame + 0.5)*pixelsPerFrame;
    const qreal endX = (getMaxFrame() - minViewedFrame + 0.5)*pixelsPerFrame;
    if(qAbs(pressX - startX) < 5) return &mMinFrame;
    else if(qAbs(pressX - endX) < 5) return &mMaxFrame;
    else if(pressX > startX && pressX < endX) return this;
    return nullptr;
}

void DurationRectangle::changeFramePosBy(const int &change) {
    mMinFrame.changeFramePosByWithoutSignal(change);
    mMaxFrame.setMinPos(getMinFrame());
    mMaxFrame.changeFramePosByWithoutSignal(change);
    mMinFrame.setMaxPos(getMaxFrame());
    DurationRectangleMovable::changeFramePosBy(change);
}

void DurationRectangle::startMinFramePosTransform() {
    mMinFrame.startPosTransform();
}

void DurationRectangle::finishMinFramePosTransform() {
    mMinFrame.finishPosTransform();
}

void DurationRectangle::moveMinFrame(const int &change) {
    mMinFrame.changeFramePosBy(change);
}

void DurationRectangle::startMaxFramePosTransform() {
    mMaxFrame.startPosTransform();
}

void DurationRectangle::openDurationSettingsDialog(QWidget *parent) {
    int oldMinFrame = getMinFrame();
    int oldMaxFrame = getMaxFrame();

    DurationRectSettingsDialog *dialog = nullptr;
    dialog = new DurationRectSettingsDialog(mType,
                                            getMinFrame(),
                                            getMaxFrame(),
                                            parent);
    if(dialog->exec()) {
        setMinFrame(dialog->getMinFrame());
        setMaxFrame(dialog->getMaxFrame());
    }

    if(dialog) {
        if(dialog->result() == QDialog::Accepted) {
            const int newMinFrame = getMinFrame();
            const int newMaxFrame = getMaxFrame();

            const int minMinFrame = qMin(oldMinFrame, newMinFrame);
            const int maxMinFrame = qMax(oldMinFrame, newMinFrame);
            const int minMaxFrame = qMin(oldMaxFrame, newMaxFrame);
            const int maxMaxFrame = qMax(oldMaxFrame, newMaxFrame);
            mChildProperty->prp_afterChangedRelRange(
                                    {minMinFrame, maxMinFrame});
            mChildProperty->prp_afterChangedRelRange(
                                    {minMaxFrame, maxMaxFrame});
        }
        delete dialog;
    }
}

void DurationRectangle::finishMaxFramePosTransform() {
    mMaxFrame.finishPosTransform();
}

void DurationRectangle::moveMaxFrame(const int &change) {
    mMaxFrame.changeFramePosBy(change);
}

int AnimationRect::getMaxAnimationFrameAsRelFrame() const {
    return getMaxAnimationFrame() - mFramePos;
}

int AnimationRect::getMinAnimationFrameAsRelFrame() const {
    return getMinAnimationFrame() - mFramePos;
}

int AnimationRect::getMaxAnimationFrameAsAbsFrame() const {
    return mChildProperty->prp_relFrameToAbsFrame(
                getMaxAnimationFrameAsRelFrame());
}

int AnimationRect::getMinAnimationFrameAsAbsFrame() const {
    return mChildProperty->prp_relFrameToAbsFrame(
                getMinAnimationFrameAsRelFrame());
}

void AnimationRect::setAnimationFrameDuration(const int &frameDuration) {
    const int oldMinFrame = getMinFrame();
    const int oldMaxFrame = getMaxFrame();
    const int oldMinAnimFrame = getMinAnimationFrame();
    const int oldMaxAnimFrame = getMaxAnimationFrame();

    setMaxAnimationFrame(getMinAnimationFrame() + frameDuration - 1);

    const int newMinFrame = getMinFrame();
    const int newMaxFrame = getMaxFrame();
    const int newMinAnimFrame = getMinAnimationFrame();
    const int newMaxAnimFrame = getMaxAnimationFrame();

    if(oldMinFrame != newMinFrame) {
        const int minMinFrame = qMin(oldMinFrame, newMinFrame);
        const int maxMinFrame = qMax(oldMinFrame, newMinFrame);
        mChildProperty->prp_afterChangedRelRange(
                                {minMinFrame + 1, maxMinFrame});
    }
    if(oldMaxFrame != newMaxFrame) {
        const int minMaxFrame = qMin(oldMaxFrame, newMaxFrame);
        const int maxMaxFrame = qMax(oldMaxFrame, newMaxFrame);
        mChildProperty->prp_afterChangedRelRange(
                                {minMaxFrame, maxMaxFrame - 1});
    }

    if(oldMinAnimFrame != newMinAnimFrame) {
        const int minMinAnimFrame = qMin(oldMinAnimFrame, newMinAnimFrame);
        const int maxMinAnimFrame = qMax(oldMinAnimFrame, newMinAnimFrame);
        mChildProperty->prp_afterChangedRelRange(
                    {minMinAnimFrame, maxMinAnimFrame - 1});
    }

    if(oldMaxAnimFrame != newMaxAnimFrame) {
        const int minMaxAnimFrame = qMin(oldMaxAnimFrame, newMaxAnimFrame);
        const int maxMaxAnimFrame = qMax(oldMaxAnimFrame, newMaxAnimFrame);
        mChildProperty->prp_afterChangedRelRange(
                    {minMaxAnimFrame + 1, maxMaxAnimFrame});
    }
}

int AnimationRect::getAnimationFrameDuration() {
    return getMaxAnimationFrame() - getMinAnimationFrame() + 1;
}

void AnimationRect::draw(QPainter * const p,
                         const QRect& drawRect,
                         const qreal &fps,
                         const qreal &pixelsPerFrame,
                         const FrameRange &absFrameRange) {
    p->save();

    const int firstRelDrawFrame =
            qMax(absFrameRange.fMin, getMinAnimationFrame()) - absFrameRange.fMin;
    const int lastRelDrawFrame =
            qMin(absFrameRange.fMax, getMaxAnimationFrame()) - absFrameRange.fMin;
    const int drawFrameSpan = lastRelDrawFrame - firstRelDrawFrame + 1;
    QRect animDurRect(qFloor(firstRelDrawFrame*pixelsPerFrame), drawRect.y(),
                      qCeil(drawFrameSpan*pixelsPerFrame), drawRect.height());

    p->fillRect(animDurRect.adjusted(0, 1, 0, -1), QColor(125, 125, 255, 180));

    DurationRectangle::draw(p, drawRect, fps, pixelsPerFrame, absFrameRange);
    p->restore();
}

int FixedLenAnimationRect::getMinAnimationFrame() const {
    return mMinAnimationFrame;
}

int FixedLenAnimationRect::getMaxAnimationFrame() const {
    return mMaxAnimationFrame;
}

void FixedLenAnimationRect::openDurationSettingsDialog(QWidget *parent) {
    int oldMinFrame = getMinFrame();
    int oldMaxFrame = getMaxFrame();
    int oldMinAnimationFrame = getMinAnimationFrame();
    int oldMaxAnimationFrame = getMaxAnimationFrame();
    DurationRectSettingsDialog *dialog = nullptr;
    dialog = new DurationRectSettingsDialog(mType,
                                            getMinFrame(),
                                            getMaxFrame(),
                                            getMinAnimationFrame(),
                                            parent);
    if(dialog->exec()) {
        setMinFrame(dialog->getMinFrame());
        setMaxFrame(dialog->getMaxFrame());
        setFirstAnimationFrame(dialog->getFirstAnimationFrame());
    }

    if(dialog) {
        if(dialog->result() == QDialog::Accepted) {
            int newMinFrame = getMinFrame();
            int newMaxFrame = getMaxFrame();
            int newMinAnimationFrame = getMinAnimationFrame();
            int newMaxAnimationFrame = getMaxAnimationFrame();
            int minMinFrame = qMin(oldMinFrame, newMinFrame);
            int maxMinFrame = qMax(oldMinFrame, newMinFrame);
            int minMaxFrame = qMin(oldMaxFrame, newMaxFrame);
            int maxMaxFrame = qMax(oldMaxFrame, newMaxFrame);
            mChildProperty->prp_afterChangedRelRange(
                        {minMinFrame, maxMinFrame});
            mChildProperty->prp_afterChangedRelRange(
                        {minMaxFrame, maxMaxFrame});

            int minMinAnimationFrame = qMin(oldMinAnimationFrame,
                                            newMinAnimationFrame);
            int maxMinAnimationFrame = qMax(oldMinAnimationFrame,
                                            newMinAnimationFrame);
            int minMaxAnimationFrame = qMin(oldMaxAnimationFrame,
                                            newMaxAnimationFrame);
            int maxMaxAnimationFrame = qMax(oldMaxAnimationFrame,
                                            newMaxAnimationFrame);
            mChildProperty->prp_afterChangedRelRange(
                        {qMin(minMinAnimationFrame, minMaxAnimationFrame),
                        qMax(maxMinAnimationFrame, maxMaxAnimationFrame)});
        }
        delete dialog;
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

void FixedLenAnimationRect::setMinAnimationFrame(const int &minAnimationFrame) {
    mMinAnimationFrame = minAnimationFrame;
    if(mBoundToAnimation) bindToAnimationFrameRange();
}

void FixedLenAnimationRect::setMaxAnimationFrame(const int &maxAnimationFrame) {
    bool moveMaxPosFrame = getMinFrame() == mFramePos &&
                           getMaxFrame() == mFramePos + getAnimationFrameDuration();
    mMaxAnimationFrame = maxAnimationFrame;
    if(mSetMaxFrameAtLeastOnce) {
        if(moveMaxPosFrame) {
            setMaxFrame(mMaxAnimationFrame);
        }
    } else {
        mSetMaxFrameAtLeastOnce = true;
        setMaxFrame(maxAnimationFrame);
        mMinFrame.setMaxPos(maxAnimationFrame);
    }
    if(mBoundToAnimation) {
        bindToAnimationFrameRange();
    }
}

void FixedLenAnimationRect::changeFramePosBy(const int &change) {
    mMaxAnimationFrame += change;
    mMinAnimationFrame += change;
    mMinFrame.changeFramePosByWithoutSignal(change);
    mMaxFrame.setMinPos(getMinFrame());
    mMaxFrame.changeFramePosByWithoutSignal(change);
    mMinFrame.setMaxPos(getMaxFrame());
    if(mBoundToAnimation) bindToAnimationFrameRange();
    DurationRectangleMovable::changeFramePosBy(change);
}
