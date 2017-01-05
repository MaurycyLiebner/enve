#ifndef QREALANIMATORITEMWIDGET_H
#define QREALANIMATORITEMWIDGET_H

#include <QWidget>
#include "connectedtomainwindow.h"
class QrealAnimator;

class QrealAnimatorItemWidget : public QWidget, public ConnectedToMainWindow
{
    Q_OBJECT
public:
    QrealAnimatorItemWidget(QrealAnimator *target, QWidget *parent = 0);
    virtual void draw(QPainter *p);

    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *event);

    void drawKeys(QPainter *p, qreal pixelsPerFrame,
                  int animatorTop, int maxY,
                  int minViewedFrame, int maxViewedFrame);

    QrealAnimator *getTargetAnimator() { return mTargetAnimator; }
    void mouseMoveEvent(QMouseEvent *);
protected:
    QrealAnimator *mTargetAnimator;
};

#endif // QREALANIMATORITEMWIDGET_H
