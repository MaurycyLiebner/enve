#ifndef DURATIONRECTANGLE_H
#define DURATIONRECTANGLE_H
#include <QtCore>
#include <QPainter>
#include <QObject>
#include "smartPointers/sharedpointerdefs.h"
#include "framerange.h"
class Property;

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
    DurationRectangleMovable(const Type &type);
    DurationRectangleMovable() {}

    void setFramePos(const int &framePos);

    int getFramePos() const;

    virtual DurationRectangleMovable *getMovableAt(
                      const int &pressX,
                      const qreal &pixelsPerFrame,
                      const int &minViewedFrame);

    virtual void changeFramePosBy(const int &change);

    void changeFramePosByWithoutSignal(const int &change);

    void setHovered(const bool &hovered);
    bool isHovered();
    virtual Qt::CursorShape getHoverCursorShape() {
        return Qt::SplitHCursor;
    }

    void pressed(const bool &shiftPressed);

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
    void setType(const Type &type) {
        mType = type;
    }

    const Type &getType() {
        return mType;
    }
public slots:
    void setMaxPos(const int &maxPos);

    void setMinPos(const int &minPos);
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
    DurationRectangle(Property *childProp);

    int getFrameShift() {
        return getFramePos();
    }

    void setFramesDuration(const int &duration);

    virtual void setMinFrame(const int &minFrame);

    virtual void setMaxFrame(const int &maxFrame);

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

    virtual void draw(QPainter * const p,
                      const QRect &drawRect,
                      const qreal &pixelsPerFrame,
                      const FrameRange &absFrameRange);

    virtual DurationRectangleMovable *getMovableAt(
                      const int &pressX,
                      const qreal &pixelsPerFrame,
                      const int &minViewedFrame);

    void changeFramePosBy(const int &change);

    Qt::CursorShape getHoverCursorShape() {
        return Qt::OpenHandCursor;
    }

    virtual bool hasAnimationFrameRange() { return false; }
    virtual void writeDurationRectangle(QIODevice *target);
    virtual void readDurationRectangle(QIODevice *target);

    void moveMinFrame(const int &change);
    void finishMinFramePosTransform();
    void startMinFramePosTransform();
    void moveMaxFrame(const int &change);
    void finishMaxFramePosTransform();
    void startMaxFramePosTransform();

    virtual void openDurationSettingsDialog(QWidget *parent = nullptr);
signals:
    void minFrameChangedBy(int);
    void maxFrameChangedBy(int);
    void rangeChanged();
    void finishedRangeChange();
protected:
    DurationRectangleMovable mMinFrame;
    DurationRectangleMovable mMaxFrame;
};

class AnimationRect : public DurationRectangle {
    Q_OBJECT
public:
    AnimationRect(Property *childProp) : DurationRectangle(childProp) {}

    bool hasAnimationFrameRange() { return true; }

    void draw(QPainter * const p,
              const QRect &drawRect,
              const qreal &pixelsPerFrame,
              const FrameRange &absFrameRange);

    virtual int getMinAnimationFrame() const = 0;
    virtual int getMaxAnimationFrame() const = 0;

    int getMaxAnimationFrameAsRelFrame() const;
    int getMinAnimationFrameAsRelFrame() const;

    int getMaxAnimationFrameAsAbsFrame() const;
    int getMinAnimationFrameAsAbsFrame() const;

    void setAnimationFrameDuration(const int &frameDuration);

    int getAnimationFrameDuration();
signals:
protected:
    virtual void setMinAnimationFrame(const int &minAnimationFrame) = 0;
    virtual void setMaxAnimationFrame(const int &maxAnimationFrame) = 0;
};

class VaryingLenAnimationRect : public AnimationRect {
    Q_OBJECT
public:
    VaryingLenAnimationRect(Property *childProp) : AnimationRect(childProp) {
        mType = VARYING_LEN_ANIMATION_RECT;
    }

    int getMinAnimationFrame() const {
        return getMinFrame();
    }

    int getMaxAnimationFrame() const {
        return getMaxFrame();
    }
protected:
    void setMinAnimationFrame(const int &minAnimationFrame) {
        setMinFrame(minAnimationFrame);
    }

    void setMaxAnimationFrame(const int &maxAnimationFrame) {
        setMaxFrame(maxAnimationFrame);
    }
};

class FixedLenAnimationRect : public AnimationRect {
    Q_OBJECT
public:
    FixedLenAnimationRect(Property *childProp) : AnimationRect(childProp) {
        mType = FIXED_LEN_ANIMATION_LEN;
    }

    int getMinAnimationFrame() const;
    int getMaxAnimationFrame() const;

    void bindToAnimationFrameRange();
    void setBindToAnimationFrameRange();
    void changeFramePosBy(const int &change);
    void writeDurationRectangle(QIODevice *target);
    void readDurationRectangle(QIODevice *target);

    void setFirstAnimationFrame(const int &minAnimationFrame) {
        int animDur = mMaxAnimationFrame - mMinAnimationFrame;
        setMinAnimationFrame(minAnimationFrame);
        setMaxAnimationFrame(minAnimationFrame + animDur);
    }
    void openDurationSettingsDialog(QWidget *parent = nullptr);
protected:
    void setMinAnimationFrame(const int &minAnimationFrame);
    void setMaxAnimationFrame(const int &maxAnimationFrame);

    bool mBoundToAnimation = false;
    bool mSetMaxFrameAtLeastOnce = false;
    int mMinAnimationFrame = 0;
    int mMaxAnimationFrame = 100;
};

#endif // DURATIONRECTANGLE_H
