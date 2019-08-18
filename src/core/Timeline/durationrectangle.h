#ifndef DURATIONRECTANGLE_H
#define DURATIONRECTANGLE_H
#include <QtCore>
#include <QPainter>
#include <QObject>
#include "smartPointers/ememory.h"
#include "framerange.h"
#include "Properties/property.h"
class Property;
class HDDCachableCacheHandler;

class TimelineMovable : public SelfRef {
    Q_OBJECT
public:
    enum Type {
        MIN_FRAME,
        MAX_FRAME,
        DURATION_RECT
    };
    TimelineMovable(const Type type, Property& parentProp);

    virtual void pressed(const bool shiftPressed) {
        Q_UNUSED(shiftPressed);
    }

    virtual Qt::CursorShape getHoverCursorShape() {
        return Qt::ArrowCursor;
    }

    virtual TimelineMovable *getMovableAt(
            const int pressX, const qreal pixelsPerFrame,
            const int minViewedFrame);

    void setValue(const int value);
    int getValue() const;

    void changeFramePosBy(const int change);

    void setHovered(const bool hovered);
    bool isHovered();

    void startPosTransform() {}
    void finishPosTransform() {}

    bool isDurationRect() { return mType == DURATION_RECT; }
    bool isMinFrame() { return mType == MIN_FRAME; }
    bool isMaxFrame() { return mType == MAX_FRAME; }

    Property* getParentProperty() const { return &mParentProperty; }

    void setClamp(const int min, const int max) {
        setClampMin(min);
        setClampMax(max);
    }

    void setClampMin(const int min);
    void setClampMax(const int max);
signals:
    void posChangedBy(int);
    void posChanged(int);
    void finishedTransform();
protected:
    const Type mType;
    Property& mParentProperty;

    bool mHovered = false;
    int mClampMin = 0;
    int mClampMax = 0;
    int mValue = 0;
};

class DurationMinMax : public TimelineMovable {
public:
    DurationMinMax(const Type type, Property& parentProp) :
        TimelineMovable(type, parentProp) {}

    Qt::CursorShape getHoverCursorShape() {
        return Qt::SplitHCursor;
    }

    void setRelFrame(const int relFrame) {
        setValue(relFrame);
    }

    int getRelFrame() const {
        return getValue();
    }

    void setAbsFrame(const int absFrame) {
        const int relFrame = mParentProperty.prp_absFrameToRelFrame(absFrame);
        setRelFrame(relFrame);
    }

    int getAbsFrame() const {
        const int absFrame = mParentProperty.prp_relFrameToAbsFrame(getRelFrame());
        return absFrame;
    }
private:
    using TimelineMovable::setValue;
    using TimelineMovable::getValue;
};

class DurationRectangle : public TimelineMovable {
    Q_OBJECT
public:
    DurationRectangle(Property &parentProp);

    virtual void draw(QPainter * const p,
                      const QRect &drawRect,
                      const qreal fps,
                      const qreal pixelsPerFrame,
                      const FrameRange &absFrameRange);

    virtual TimelineMovable *getMovableAt(
                      const int pressX,
                      const qreal pixelsPerFrame,
                      const int minViewedFrame);
    virtual bool hasAnimationFrameRange() { return false; }
    virtual void writeDurationRectangle(QIODevice *dst);
    virtual void readDurationRectangle(QIODevice *src);

    void openDurationSettingsDialog(QWidget *parent = nullptr);

    void pressed(const bool shiftPressed);

    Qt::CursorShape getHoverCursorShape() {
        return Qt::OpenHandCursor;
    }

    bool isSelected();

    void setFramesDuration(const int duration);
    int getFrameDuration() const;

    void setMinRelFrame(const int minFrame);
    void setMaxRelFrame(const int maxFrame);

    void setMinAbsFrame(const int minFrame);
    void setMaxAbsFrame(const int maxFrame);

    int getMinRelFrame() const;
    int getMaxRelFrame() const;

    int getMinAbsFrame() const;
    int getMaxAbsFrame() const;

    FrameRange getRelFrameRange() const{
        return {getMinRelFrame(), getMaxRelFrame()};
    }

    FrameRange getRelFrameRangeToTheRight() {
        return {getMaxRelFrame() + 1, FrameRange::EMAX};
    }

    FrameRange getRelFrameRangeToTheLeft() {
        return {FrameRange::EMIN, getMinRelFrame() - 1};
    }

    FrameRange getAbsFrameRange() const{
        return {getMinAbsFrame(), getMaxAbsFrame()};
    }

    FrameRange getAbsFrameRangeToTheRight() {
        return {getMaxAbsFrame() + 1, FrameRange::EMAX};
    }

    FrameRange getAbsFrameRangeToTheLeft() {
        return {FrameRange::EMIN, getMinAbsFrame() - 1};
    }

    void moveMinFrame(const int change);
    void finishMinFramePosTransform();
    void startMinFramePosTransform();

    void moveMaxFrame(const int change);
    void finishMaxFramePosTransform();
    void startMaxFramePosTransform();

    void setRasterCacheHandler(const HDDCachableCacheHandler * const handler) {
        mRasterCacheHandler = handler;
    }

    void setSoundCacheHandler(const HDDCachableCacheHandler * const handler) {
        mSoundCacheHandler = handler;
    }

    void setRelShift(const int shift) {
        setValue(shift);
    }

    int getRelShift() const { return getValue(); }
private:
    using TimelineMovable::setValue;
    using TimelineMovable::getValue;
signals:
    void minFrameChangedBy(int);
    void maxFrameChangedBy(int);
    void rangeChanged();
    void finishedRangeChange();
protected:
    const HDDCachableCacheHandler * mRasterCacheHandler = nullptr;
    const HDDCachableCacheHandler * mSoundCacheHandler = nullptr;
    DurationMinMax mMinFrame;
    DurationMinMax mMaxFrame;
};

#endif // DURATIONRECTANGLE_H
