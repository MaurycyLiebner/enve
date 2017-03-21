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

class DurationRectangle : public DurationRectangleMovable {
    Q_OBJECT
public:
    DurationRectangle();

    void setFramesDuration(const int &duration);

    void setMinFrame(const int &minFrame);

    void setMaxFrame(const int &maxFrame);

    int getFrameDuration() const;

    int getMinFrame() const;

    int getMaxFrame() const;

    void draw(QPainter *p,
              const qreal &pixelsPerFrame,
              const qreal &drawY,
              const int &startFrame);

    virtual DurationRectangleMovable *getMovableAt(
                      const int &pressX,
                      const qreal &pixelsPerFrame,
                      const int &minViewedFrame);

    void changeFramePosBy(const int &change);

    void setPossibleFrameRangeVisible();

    int getMinPossibleFrame();

    int getMaxPossibleFrame();

    void setMinPossibleFrame(const int &minPossibleFrame);

    void setMaxPossibleFrame(const int &maxPossibleFrame);

    void setPossibleFrameDuration(const int &frameDuration);

    int getPossibleFrameDuration();

    Qt::CursorShape getHoverCursorShape() {
        return Qt::OpenHandCursor;
    }
signals:
    void changed();
protected:
    bool mSetMaxFrameAtLeastOnce = false;
    bool mShowPossibleFrameRange = false;
    int mMinPossibleFrame = 0;
    int mMaxPossibleFrame = 100;
    DurationRectangleMovable mMinFrame;
    DurationRectangleMovable mMaxFrame;
};

#endif // DURATIONRECTANGLE_H
