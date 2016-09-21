#ifndef ANIMATONWIDGETSCROLLBAR_H
#define ANIMATONWIDGETSCROLLBAR_H

#include <QWidget>

class AnimatonWidgetScrollBar : public QWidget
{
    Q_OBJECT
public:
    explicit AnimatonWidgetScrollBar(int minSpan, int maxSpan,
                                     int spanInc, int height, bool range,
                                     bool clamp,
                                     QWidget *parent = 0);
    void emitChange();
    bool setFirstViewedFrame(int firstFrame);
    void setFramesSpan(int newSpan);

    int getMaxFrame();
    int getMinFrame();
protected:
    qreal posToFrame(int xPos);
    void paintEvent(QPaintEvent *);
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);
signals:
    void viewedFramesChanged(int, int);
public slots:
    void setViewedFramesRange(int startFrame, int endFrame);

    void setMinMaxFrames(int minFrame, int maxFrame);
private:
    bool mClamp;

    bool mRange;
    bool mPressed = false;

    qreal mLastMousePressFrame;
    int mSavedFirstFrame;

    int mFirstViewedFrame = 0;
    int mFramesSpan = 50;

    int mMinFrame = 0;
    int mMaxFrame = 200;

    int mMinSpan;
    int mMaxSpan;
    int mSpanInc;
};

#endif // ANIMATONWIDGETSCROLLBAR_H
