#ifndef COMPLEXANIMATORITEMWIDGET_H
#define COMPLEXANIMATORITEMWIDGET_H
#include "qrealanimatoritemwidget.h"

class ComplexAnimatorItemWidget : public QrealAnimatorItemWidget
{
    Q_OBJECT
public:
    ComplexAnimatorItemWidget(QrealAnimator *target,
                              QWidget *parent = 0);
    void draw(QPainter *p);

    void mousePressEvent(QMouseEvent *event);

    void mouseMoveEvent(QMouseEvent *event);

    void setDetailsVisibile(bool bT);
signals:
    void detailsVisibilityChanged(bool);
private:
    bool mDetailsVisible = false;
};

#endif // COMPLEXANIMATORITEMWIDGET_H
