#ifndef EFFECTANIMATORSITEMWIDGETCONTAINER_H
#define EFFECTANIMATORSITEMWIDGETCONTAINER_H
#include "complexanimatoritemwidgetcontainer.h"

class EffectAnimatorsItemWidgetContainer :
        public ComplexAnimatorItemWidgetContainer
{
public:
    EffectAnimatorsItemWidgetContainer(QrealAnimator *target,
                                       QWidget *parent = 0);

protected:
    void dragMoveEvent(QDragMoveEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *);

    QWidget *mDragHighlightWidget = NULL;
};

#endif // EFFECTANIMATORSITEMWIDGETCONTAINER_H
