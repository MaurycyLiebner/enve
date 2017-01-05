#ifndef BOXITEMWIDGETCONTAINER_H
#define BOXITEMWIDGETCONTAINER_H
#include "widgetcontainer.h"
class QrealKey;
class BoundingBox;
class BoxItemWidget;
class ComplexAnimatorItemWidgetContainer;
class QrealAnimator;

class BoxItemWidgetContainer : public WidgetContainer
{
    Q_OBJECT
public:
    BoxItemWidgetContainer(BoundingBox *target, QWidget *parent = 0);

    static BoxItemWidgetContainer *createBoxItemWidgetContainer(
                                                          BoundingBox *target,
                                                          QWidget *parent = 0);

    BoundingBox *getTargetBox();

    void initialize();

    void drawKeys(QPainter *p, qreal pixelsPerFrame,
                  int containerTop, int maxY,
                  int minViewedFrame, int maxViewedFrame);
    QrealKey *getKeyAtPos(qreal pressX, qreal pressY, qreal pixelsPerFrame, int maxY,
                          int containerTop, int minViewedFrame);
    void getKeysInRect(QRectF selectionRect, int containerTop,
                       qreal pixelsPerFrame, int minViewedFrame,
                       QList<QrealKey*> *listKeys);
public slots:
    void addAnimatorWidgetForAnimator(QrealAnimator *animator);
    void removeAnimatorWidgetForAnimator(QrealAnimator *animator);
protected:
    void addAnimatorWidget(ComplexAnimatorItemWidgetContainer *widget);
    void removeAnimatorWidget(ComplexAnimatorItemWidgetContainer *widget);

//    void dragEnterEvent(QDragEnterEvent *event);
//    void dragMoveEvent(QDragMoveEvent *event);
//    void dropEvent(QDropEvent *event);

    BoxItemWidget *mTargetBoxWidget = NULL;
    QList<ComplexAnimatorItemWidgetContainer*> mAnimatorsContainers;
};

#endif // BOXITEMWIDGETCONTAINER_H
