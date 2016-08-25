#ifndef ANIMATONWIDGETSCROLLBAR_H
#define ANIMATONWIDGETSCROLLBAR_H

#include <QWidget>

class AnimatonWidgetScrollBar : public QWidget
{
    Q_OBJECT
public:
    explicit AnimatonWidgetScrollBar(QWidget *parent = 0);
    void emitChange();
protected:
    int frameToPos(int frame);
    int posToFrame(int xPos);
    void paintEvent(QPaintEvent *);
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *);
signals:
    void viewedFramesChanged(int, int);
public slots:
    void setViewedFramesRange(int startFrame, int endFrame);
private:
    bool mPressed = false;

    int mLastMouseEventFrame;
    int mSavedFirstFrame;

    int mFirstViewedFrame = 0;
    int mFramesSpan = 50;

    int mMinFrame = 0;
    int mMaxFrame = 200;
};

#endif // ANIMATONWIDGETSCROLLBAR_H
