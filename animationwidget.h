#ifndef ANIMATIONWIDGET_H
#define ANIMATIONWIDGET_H

#include <QWidget>
#include "qrealanimator.h"

class AnimationWidget : public QWidget
{
    Q_OBJECT
public:
    explicit AnimationWidget(QWidget *parent = 0);
protected:
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);
signals:
    void changedViewedFrames(int, int);
public slots:
    void mergeKeysIfNeeded();
    void updateDrawnPath();
    void setViewedFramesRange(int startFrame, int endFrame);

    void setTwoSizeCtrl();
    void setRightSideCtrl();
    void setLeftSideCtrl();
    void setNoSideCtrl();
    void setSmoothCtrl();
    void setSymmetricCtrl();
    void setCornerCtrl();
private:
    QrealAnimator *mAnimator;
};

#endif // ANIMATIONWIDGET_H
