#ifndef DURATIONRECTANGLE_H
#define DURATIONRECTANGLE_H
#include <QtCore>
#include <QPainter>
#include <QObject>
#include "smartPointers/sharedpointerdefs.h"
#include "framerange.h"
class Property;
class HDDCachableCacheHandler;

class DurationRectangleMovable : public SelfRef {
    Q_OBJECT
public:
    enum Type {
        MIN_FRAME,
        MAX_FRAME,
        DURATION_RECT,
        VARYING_LEN_ANIMATION_RECT,
        FIXED_LEN_ANIMATION_LEN,
        NOT_SPECIFIED
    };
    DurationRectangleMovable(const Type type);
    DurationRectangleMovable() {}

    void setFramePos(const int framePos);

    int getFramePos() const;

    virtual DurationRectangleMovable *getMovableAt(
                      const int pressX,
                      const qreal pixelsPerFrame,
                      const int minViewedFrame);

    virtual void changeFramePosBy(const int change);

    void changeFramePosByWithoutSignal(const int change);

    void setHovered(const bool hovered);
    bool isHovered();
    virtual Qt::CursorShape getHoverCursorShape() {
        return Qt::SplitHCursor;
    }

    void pressed(const bool shiftPressed);

    void setChildProperty(Property * const childProp) {
        mChildProperty = childProp;
    }

    void startPosTransform() {}
    void finishPosTransform() {}

    bool isSelected();

    bool isDurationRect() {
        return mType == DURATION_RECT ||
               mType == VARYING_LEN_ANIMATION_RECT ||
               mType == FIXED_LEN_ANIMATION_LEN;
    }
    bool isMinFrame() {
        return mType == MIN_FRAME;
    }

    bool isMaxFrame() {
        return mType == MAX_FRAME;
    }

    void setType(const Type type) {
        mType = type;
    }

    const Type &getType() {
        return mType;
    }

    Property * getChildProperty() {
        return mChildProperty;
    }

    void setMinPos(const int minPos);
    void setMaxPos(const int maxPos);
signals:
    void posChangedBy(int);
    void posChanged(int);
    void finishedTransform();
protected:
    bool mHovered = false;
    Type mType = NOT_SPECIFIED;
    int mMinPos = 0;
    int mMaxPos = 0;
    int mFramePos = 0;
    Property *mChildProperty = nullptr;
};

class DurationRectangle : public DurationRectangleMovable {
    Q_OBJECT
public:
    DurationRectangle(Property * const childProp);

    virtual void setMinFrame(const int minFrame);
    virtual void setMaxFrame(const int maxFrame);

    virtual void draw(QPainter * const p,
                      const QRect &drawRect,
                      const qreal fps,
                      const qreal pixelsPerFrame,
                      const FrameRange &absFrameRange);

    virtual DurationRectangleMovable *getMovableAt(
                      const int pressX,
                      const qreal pixelsPerFrame,
                      const int minViewedFrame);
    virtual bool hasAnimationFrameRange() { return false; }
    virtual void writeDurationRectangle(QIODevice *dst);
    virtual void readDurationRectangle(QIODevice *src);
    virtual void openDurationSettingsDialog(QWidget *parent = nullptr);

    void changeFramePosBy(const int change);

    Qt::CursorShape getHoverCursorShape() {
        return Qt::OpenHandCursor;
    }

    int getFrameShift() {
        return getFramePos();
    }

    void setFramesDuration(const int duration);

    int getFrameDuration() const;

    int getMinFrame() const;
    int getMaxFrame() const;

    int getMinFrameAsRelFrame() const;
    int getMaxFrameAsRelFrame() const;

    FrameRange getRelFrameRange() const{
        return {getMinFrameAsRelFrame(), getMaxFrameAsRelFrame()};
    }

    FrameRange getRelFrameRangeToTheRight() {
        return {getMaxFrameAsRelFrame() + 1, FrameRange::EMAX};
    }

    FrameRange getRelFrameRangeToTheLeft() {
        return {FrameRange::EMIN, getMinFrameAsRelFrame() - 1};
    }

    int getMinFrameAsAbsFrame() const;
    int getMaxFrameAsAbsFrame() const;

    FrameRange getAbsFrameRange() const{
        return {getMinFrameAsAbsFrame(), getMaxFrameAsAbsFrame()};
    }

    FrameRange getAbsFrameRangeToTheRight() {
        return {getMaxFrameAsAbsFrame() + 1, FrameRange::EMAX};
    }

    FrameRange getAbsFrameRangeToTheLeft() {
        return {FrameRange::EMIN, getMinFrameAsAbsFrame() - 1};
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
signals:
    void minFrameChangedBy(int);
    void maxFrameChangedBy(int);
    void rangeChanged();
    void finishedRangeChange();
protected:
    const HDDCachableCacheHandler * mRasterCacheHandler = nullptr;
    const HDDCachableCacheHandler * mSoundCacheHandler = nullptr;
    DurationRectangleMovable mMinFrame;
    DurationRectangleMovable mMaxFrame;
};

#endif // DURATIONRECTANGLE_H
