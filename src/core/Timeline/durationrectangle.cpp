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

#include "durationrectangle.h"
#include "CacheHandlers/hddcachablecachehandler.h"
#include "GUI/global.h"
#include "Boxes/boundingbox.h"
#include "GUI/durationrectsettingsdialog.h"

TimelineMovable::TimelineMovable(const Type type, Property &parentProp) :
    mType(type), mParentProperty(parentProp) {}


void TimelineMovable::setValueUnClamped(const int value) {
    const int oldValue = mValue;
    mValue = value;
    emit valueChanged(oldValue, mValue);
}

void TimelineMovable::setValue(const int value) {
    setValueUnClamped(qMin(mClampMax, qMax(mClampMin, value)));
}

int TimelineMovable::getValue() const {
    return mValue;
}

TimelineMovable *TimelineMovable::getMovableAt(
        const int pressX, const qreal pixelsPerFrame,
        const int minViewedFrame) {
    const int pressedDFrame = qRound(minViewedFrame + pressX/pixelsPerFrame);
    const int thisDFrame = mValue - minViewedFrame;
    if(thisDFrame == pressedDFrame) return this;
    return nullptr;
}

void TimelineMovable::changeFramePosBy(const int change) {
    setValue(mValue + change);
}

void TimelineMovable::setHovered(const bool hovered) {
    mHovered = hovered;
}

bool TimelineMovable::isHovered() {
    return mHovered;
}

void TimelineMovable::setClampMax(const int max) {
    mClampMax = max;
}

void TimelineMovable::setClampMin(const int min) {
    mClampMin = min;
}

DurationRectangle::DurationRectangle(Property &parentProp) :
    TimelineMovable(DURATION_RECT, parentProp),
    mMinFrame(MIN_FRAME, parentProp),
    mMaxFrame(MAX_FRAME, parentProp) {
    connect(this, &TimelineMovable::valueChanged,
            this, &DurationRectangle::shiftChanged);

    setClamp(-1000000, 1000000);
    mMinFrame.setClamp(-1000000, 1000000);
    mMaxFrame.setClamp(-1000000, 1000000);

    connect(&mMinFrame, &TimelineMovable::valueChanged,
            this, [this](const int, const int to) {
        mMaxFrame.setClampMin(to);
    });
    connect(&mMaxFrame, &TimelineMovable::valueChanged,
            this, [this](const int, const int to) {
        mMinFrame.setClampMax(to);
    });

    connect(&mMinFrame, &TimelineMovable::valueChanged,
            this, &DurationRectangle::minRelFrameChanged);
    connect(&mMaxFrame, &TimelineMovable::valueChanged,
            this, &DurationRectangle::maxRelFrameChanged);
}

void DurationRectangle::pressed(const bool shiftPressed) {
    Q_UNUSED(shiftPressed)
//    if(mParentProperty.SWT_isBoundingBox() ||
//       mParentProperty.SWT_isSingleSound()) {
//        const auto cont = static_cast<eBoxOrSound*>(&mParentProperty);
//        cont->selectionChangeTriggered(shiftPressed);
//    }
}

bool DurationRectangle::isSelected() {
    if(mParentProperty.SWT_isBoundingBox() ||
       mParentProperty.SWT_isSingleSound()) {
        const auto cont = static_cast<eBoxOrSound*>(&mParentProperty);
        return cont->isSelected();
    }
    return false;
}

void DurationRectangle::setFramesDuration(const int duration) {
    mMaxFrame.setRelFrame(getMinRelFrame() + duration - 1);
}

void DurationRectangle::setMinRelFrame(const int minFrame) {
    mMinFrame.setRelFrame(minFrame);
}

void DurationRectangle::setMaxRelFrame(const int maxFrame) {
    mMaxFrame.setRelFrame(maxFrame);
}

void DurationRectangle::setMinAbsFrame(const int minFrame) {
    mMinFrame.setAbsFrame(minFrame);
}

void DurationRectangle::setMaxAbsFrame(const int maxFrame) {
    mMaxFrame.setAbsFrame(maxFrame);
}

int DurationRectangle::getFrameDuration() const {
    return mMaxFrame.getRelFrame() - mMinFrame.getRelFrame() + 1;
}

int DurationRectangle::getMinRelFrame() const {
    return mMinFrame.getRelFrame();
}

int DurationRectangle::getMaxRelFrame() const {
    return mMaxFrame.getRelFrame();
}

int DurationRectangle::getMinAbsFrame() const {
    return mMinFrame.getAbsFrame();
}

int DurationRectangle::getMaxAbsFrame() const {
    return mMaxFrame.getAbsFrame();
}

#include "Boxes/animationbox.h"
#include "Sound/singlesound.h"
void DurationRectangle::draw(QPainter * const p,
                             const QRect& drawRect,
                             const qreal fps,
                             const qreal pixelsPerFrame,
                             const FrameRange &absFrameRange) {
    const int clampedMin = qMax(absFrameRange.fMin, getMinAbsFrame());
    const int firstRelDrawFrame = clampedMin - absFrameRange.fMin;
    const int clampedMax = qMin(absFrameRange.fMax, getMaxAbsFrame());
    const int lastRelDrawFrame = clampedMax - absFrameRange.fMin;
    const int drawFrameSpan = lastRelDrawFrame - firstRelDrawFrame + 1;

    if(drawFrameSpan < 1) return;

    const QRect durRect(qFloor(firstRelDrawFrame*pixelsPerFrame), drawRect.y(),
                        qCeil(drawFrameSpan*pixelsPerFrame), drawRect.height());

    const int rectStartFrame = absFrameRange.fMin - mValue;
    const int rectEndFrame = absFrameRange.fMax - mValue;
    if(mRasterCacheHandler && mSoundCacheHandler) {
        const int soundHeight = drawRect.height()/3;
        const int rasterHeight = drawRect.height() - soundHeight;
        const QRect rasterRect(drawRect.x(), drawRect.y(),
                               drawRect.width(), rasterHeight);
        mRasterCacheHandler->drawCacheOnTimeline(p, rasterRect,
                                                 rectStartFrame,
                                                 rectEndFrame,
                                                 1,
                                                 durRect.right());
        const QRect soundRect(drawRect.x(), drawRect.y() + rasterHeight,
                              drawRect.width(), soundHeight);
        mSoundCacheHandler->drawCacheOnTimeline(p, soundRect,
                                                rectStartFrame,
                                                rectEndFrame, fps,
                                                durRect.right());
    } else if(mRasterCacheHandler) {
        mRasterCacheHandler->drawCacheOnTimeline(p, drawRect,
                                                 rectStartFrame,
                                                 rectEndFrame,
                                                 1,
                                                 durRect.right());
    } else if(mSoundCacheHandler) {
        mSoundCacheHandler->drawCacheOnTimeline(p, drawRect,
                                                rectStartFrame,
                                                rectEndFrame, fps,
                                                durRect.right());
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
}

TimelineMovable *DurationRectangle::getMovableAt(
        const int pressX, const qreal pixelsPerFrame,
        const int minViewedFrame) {
    const qreal startX = (getMinAbsFrame() - minViewedFrame)*pixelsPerFrame;
    const qreal endX = (getMaxAbsFrame() - minViewedFrame + 1)*pixelsPerFrame;
    if(qAbs(pressX - startX) < 5) return &mMinFrame;
    else if(qAbs(pressX - endX) < 5) return &mMaxFrame;
    else if(pressX > startX && pressX < endX) return this;
    return nullptr;
}

void DurationRectangle::startMinFramePosTransform() {
    mMinFrame.startPosTransform();
}

void DurationRectangle::finishMinFramePosTransform() {
    mMinFrame.finishPosTransform();
}

void DurationRectangle::moveMinFrame(const int change) {
    mMinFrame.changeFramePosBy(change);
}

void DurationRectangle::startMaxFramePosTransform() {
    mMaxFrame.startPosTransform();
}

void DurationRectangle::openDurationSettingsDialog(QWidget *parent) {
    DurationRectSettingsDialog dialog(getRelShift(),
                                      getMinRelFrame(),
                                      getMaxRelFrame(),
                                      parent);
    if(dialog.exec()) {
        if(getRelShift() == dialog.getShift() &&
           getMinRelFrame() == dialog.getMinFrame() &&
           getMaxRelFrame() == dialog.getMaxFrame()) {
            return;
        }

        const auto oldRelRange = getRelFrameRange();
        mMinFrame.setValueUnClamped(dialog.getMinFrame());
        mMaxFrame.setValueUnClamped(dialog.getMaxFrame());
        setRelShift(dialog.getShift());
        const auto newRelRange = getRelFrameRange();

        mParentProperty.prp_afterChangedRelRange(oldRelRange + newRelRange);
    }
}

void DurationRectangle::finishMaxFramePosTransform() {
    mMaxFrame.finishPosTransform();
}

void DurationRectangle::moveMaxFrame(const int change) {
    mMaxFrame.changeFramePosBy(change);
}

void DurationRectangle::writeDurationRectangle(eWriteStream &dst) {
    dst << getMinRelFrame();
    dst << getMaxRelFrame();
    dst << getRelShift();
}

void DurationRectangle::readDurationRectangle(eReadStream& src) {
    int minFrame;
    int maxFrame;
    int shift;
    src >> minFrame;
    src >> maxFrame;
    src >> shift;
    mMinFrame.setValueUnClamped(minFrame);
    mMaxFrame.setValueUnClamped(maxFrame);
    setRelShift(shift);
}
