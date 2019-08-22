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
    Canvas* getParentScene() const { return mParentScene; }
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

    void writeProperty(eWriteStream& dst) const;
    void readProperty(eReadStream& src);

    TimelineMovable *anim_getTimelineMovable(
            const int relX, const int minViewedFrame,
            const qreal pixelsPerFrame);
    void drawTimelineControls(QPainter * const p,
                              const qreal pixelsPerFrame,
                              const FrameRange &absFrameRange,
                              const int rowHeight);

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
    void setLocked(const bool bt);
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
    Canvas* mParentScene = nullptr;
    ContainerBox* mParentGroup = nullptr;

    bool mDurationRectangleLocked = false;
    qsptr<DurationRectangle> mDurationRectangle;
signals:
    void aboutToChangeParent();
    void parentChanged(ContainerBox*);
    void aboutToChangeAncestor();
    void ancestorChanged();
    void selectionChanged(bool);
    void visibilityChanged(bool);
};

#endif // EBOXORSOUND_H
