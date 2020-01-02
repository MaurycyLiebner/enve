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

#ifndef EBOXORSOUND_H
#define EBOXORSOUND_H
#include "staticcomplexanimator.h"

class ContainerBox;
class Canvas;
class DurationRectangle;
class QMimeData;

class eBoxOrSound : public StaticComplexAnimator {
    Q_OBJECT
    e_OBJECT
public:
    eBoxOrSound(const QString& name);

    void setParentGroup(ContainerBox * const parent);
    ContainerBox *getParentGroup() const { return mParentGroup; }
    void removeFromParent_k();
    bool isAncestor(const BoundingBox * const box) const;

    virtual bool isFrameInDurationRect(const int relFrame) const;
    virtual bool isFrameFInDurationRect(const qreal relFrame) const;
    virtual void shiftAll(const int shift);

    QMimeData *SWT_createMimeData();

    FrameRange prp_relInfluenceRange() const;
    FrameRange prp_getIdenticalRelRange(const int relFrame) const;
    int prp_getRelFrameShift() const;
    void prp_afterChangedAbsRange(const FrameRange &range,
                                  const bool clip = true) {
        const auto croppedRange = clip ? prp_absInfluenceRange()*range : range;
        StaticComplexAnimator::prp_afterChangedAbsRange(croppedRange);
    }

    void prp_writeProperty(eWriteStream& dst) const;
    void prp_readProperty(eReadStream& src);

    TimelineMovable *anim_getTimelineMovable(
            const int relX, const int minViewedFrame,
            const qreal pixelsPerFrame);
    void prp_drawTimelineControls(
            QPainter * const p, const qreal pixelsPerFrame,
            const FrameRange &absFrameRange, const int rowHeight);

    void setDurationRectangle(const qsptr<DurationRectangle> &durationRect);
    bool hasDurationRectangle() const;
    void createDurationRectangle();
    bool isVisibleAndInVisibleDurationRect() const;
    void startDurationRectPosTransform();
    void finishDurationRectPosTransform();
    void moveDurationRect(const int dFrame);
    void startMinFramePosTransform();
    void finishMinFramePosTransform();
    void moveMinFrame(const int dFrame);
    void startMaxFramePosTransform();
    void finishMaxFramePosTransform();
    void moveMaxFrame(const int dFrame);
    DurationRectangle *getDurationRectangle() const;
    bool isVisibleAndInDurationRect(const int relFrame) const;
    bool isFrameFVisibleAndInDurationRect(const qreal relFrame) const;

    void setSelected(const bool select);
    void select();
    void deselect();
    bool isSelected() const { return mSelected; }
    void selectionChangeTriggered(const bool shiftPressed);

    void hide();
    void show();
    bool isVisible() const;
    void setVisibile(const bool visible);
    void switchVisible();

    void lock();
    void unlock();
    void setLocked(const bool locked);
    void switchLocked();
    bool isLocked() const;

    bool isVisibleAndUnlocked() const;

    void moveUp();
    void moveDown();
    void bringToFront();
    void bringToEnd();
    void setZListIndex(const int z) { mZListIndex = z; }
    int getZIndex() const { return mZListIndex; }
protected:
    bool mSelected = false;
    bool mInVisibleRange = true;
    bool mVisible = true;
    bool mLocked = false;
    int mZListIndex = 0;
    ContainerBox* mParentGroup = nullptr;

    bool mDurationRectangleLocked = false;
    qsptr<DurationRectangle> mDurationRectangle;
signals:
    void parentChanged(ContainerBox*);
    void aboutToChangeAncestor();
    void selectionChanged(bool);
    void visibilityChanged(bool);
    void lockedChanged(bool);
};

#endif // EBOXORSOUND_H
