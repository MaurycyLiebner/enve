#ifndef DURATIONRECTANGLE_H
#define DURATIONRECTANGLE_H
#include <QtCore>
#include <QPainter>
#include <QObject>
#include "BoxesList/boxsinglewidget.h"

class DurationRectangleMovable : public QObject {
    Q_OBJECT
public:
    DurationRectangleMovable();

    void setFramePos(const int &framePos);

    int getFramePos() const;

    int getFrameShift() const;

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

public slots:
    void setMaxPos(const int &maxPos);

    void setMinPos(const int &minPos);
signals:
    void posChanged(int);
protected:
    bool mHovered = false;
    int mMinPos = 0;
    int mMaxPos = 0;
    int mFramePos = 0;
};

class Property;

class DurationRectangle : public DurationRectangleMovable {
    Q_OBJECT
public:
    DurationRectangle(Property *childProp);

    int getFrameShift();

    void setFramesDuration(const int &duration);

    void setMinFrame(const int &minFrame);

    void setMaxFrame(const int &maxFrame);

    int getFrameDuration() const;

    int getMinFrame() const;
    int getMaxFrame() const;

    int getMinFrameAsRelFrame() const;
    int getMaxFrameAsRelFrame() const;

    int getMinFrameAsAbsFrame() const;
    int getMaxFrameAsAbsFrame() const;

    void draw(QPainter *p,
              const qreal &pixelsPerFrame,
              const qreal &drawY,
              const int &startFrame);

    virtual DurationRectangleMovable *getMovableAt(
                      const int &pressX,
                      const qreal &pixelsPerFrame,
                      const int &minViewedFrame);

    void changeFramePosBy(const int &change);

    void setAnimationFrameRangeVisible();

    int getMinAnimationFrame() const;
    int getMaxAnimationFrame() const;

    int getMaxAnimationFrameAsRelFrame() const;
    int getMinAnimationFrameAsRelFrame() const { return 0; }

    int getMaxAnimationFrameAsAbsFrame() const;
    int getMinAnimationFrameAsAbsFrame() const;

    void setAnimationFrameDuration(const int &frameDuration);

    int getAnimationFrameDuration();

    Qt::CursorShape getHoverCursorShape() {
        return Qt::OpenHandCursor;
    }

    void bindToAnimationFrameRange();
    void setBindToAnimationFrameRange();

    bool hasAnimationFrameRange() { return mShowAnimationFrameRange; }
signals:
    void changed();
protected:
    Property *mChildProperty;
    void setMinAnimationFrame(const int &minAnimationFrame);

    void setMaxAnimationFrame(const int &maxAnimationFrame);

    bool mBoundToAnimation = false;
    bool mSetMaxFrameAtLeastOnce = false;
    bool mShowAnimationFrameRange = false;
    int mMinAnimationFrame = 0;
    int mMaxAnimationFrame = 100;
    DurationRectangleMovable mMinFrame;
    DurationRectangleMovable mMaxFrame;
};

#endif // DURATIONRECTANGLE_H
