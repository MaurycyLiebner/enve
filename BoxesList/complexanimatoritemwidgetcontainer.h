#ifndef COMPLEXANIMATORITEMWIDGETCONTAINER_H
#define COMPLEXANIMATORITEMWIDGETCONTAINER_H

class QrealAnimatorItemWidget;
class QrealKey;
class QrealAnimator;
#include "widgetcontainer.h"

class ComplexAnimatorItemWidgetContainer : public WidgetContainer
{
    Q_OBJECT
public:
    ComplexAnimatorItemWidgetContainer(QrealAnimator *target,
                                       QWidget *parent = 0);

    QrealAnimator *getTargetAnimator();
    void drawKeys(QPainter *p, qreal pixelsPerFrame,
                  int animatorTop, int maxY,
                  int minViewedFrame, int maxViewedFrame);
    QrealKey *getKeyAtPos(qreal pressX, qreal pressY,
                          qreal pixelsPerFrame, int maxY,
                          int animatorTop, int minViewedFrame);
    void getKeysInRect(QRectF selectionRect, int viewedTop,
                       qreal pixelsPerFrame, int minViewedFrame,
                       QList<QrealKey*> *listKeys);
public slots:
    void addChildAnimator(QrealAnimator *animator);
    void removeChildAnimator(QrealAnimator *animator);
private:
    QrealAnimatorItemWidget *mTargetAnimatorWidget;
    QList<ComplexAnimatorItemWidgetContainer*> mChildWidgets;
};

#endif // COMPLEXANIMATORITEMWIDGETCONTAINER_H
