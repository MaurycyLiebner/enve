#ifndef DURATIONRECTANGLE_H
#define DURATIONRECTANGLE_H
#include <QtCore>
#include <QPainter>
#include <QObject>
#include <QSqlQuery>


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
    void posChangedBy(int);
    void posChanged(int);
    void finishedTransform();
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

    int getMinFrameAsAbsFrame() const;
    int getMaxFrameAsAbsFrame() const;

    virtual void draw(QPainter *p,
                      const qreal &pixelsPerFrame,
                      const qreal &drawY,
                      const int &startFrame);

    virtual DurationRectangleMovable *getMovableAt(
                      const int &pressX,
                      const qreal &pixelsPerFrame,
                      const int &minViewedFrame);

    void changeFramePosBy(const int &change);

    Qt::CursorShape getHoverCursorShape() {
        return Qt::OpenHandCursor;
    }

    virtual bool hasAnimationFrameRange() { return false; }
    void loadFromSql(const int &durRectId);
    int saveToSql(QSqlQuery *query);
signals:
    void minFrameChangedBy(int);
    void maxFrameChangedBy(int);
    void rangeChanged();
    void finishedRangeChange();
protected:
    Property *mChildProperty;

    DurationRectangleMovable mMinFrame;
    DurationRectangleMovable mMaxFrame;
};

class AnimationRect : public DurationRectangle {
    Q_OBJECT
public:
    AnimationRect(Property *childProp) : DurationRectangle(childProp) {}

    bool hasAnimationFrameRange() { return true; }

    void draw(QPainter *p,
              const qreal &pixelsPerFrame,
              const qreal &drawY,
              const int &startFrame);

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

    }

    int getMinAnimationFrame() const;
    int getMaxAnimationFrame() const;

    void bindToAnimationFrameRange();
    void setBindToAnimationFrameRange();
    void changeFramePosBy(const int &change);
protected:
    void setMinAnimationFrame(const int &minAnimationFrame);
    void setMaxAnimationFrame(const int &maxAnimationFrame);

    bool mBoundToAnimation = false;
    bool mSetMaxFrameAtLeastOnce = false;
    int mMinAnimationFrame = 0;
    int mMaxAnimationFrame = 100;
};

#endif // DURATIONRECTANGLE_H
