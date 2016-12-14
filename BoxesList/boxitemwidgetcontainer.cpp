#include "boxitemwidgetcontainer.h"
#include "boxitemwidget.h"
#include "boundingbox.h"
#include "boxeslistwidget.h"
#include "complexanimatoritemwidgetcontainer.h"

BoxItemWidgetContainer::BoxItemWidgetContainer(BoundingBox *target,
                                               QWidget *parent) :
    WidgetContainer(parent)
{
    mTargetBoxWidget = new BoxItemWidget(target, this);
    setTopWidget(mTargetBoxWidget);
    connect(mTargetBoxWidget, SIGNAL(detailsVisibilityChanged(bool)),
            mDetailsWidget, SLOT(setVisible(bool)));

    target->addAllAnimatorsToBoxItemWidgetContainer(this);

    connect(target, SIGNAL(addActiveAnimatorSignal(QrealAnimator*)),
            this, SLOT(addAnimatorWidgetForAnimator(QrealAnimator*)));
    connect(target, SIGNAL(removeActiveAnimatorSignal(QrealAnimator*)),
            this, SLOT(removeAnimatorWidgetForAnimator(QrealAnimator*)));
}

BoundingBox *BoxItemWidgetContainer::getTargetBox() {
    return mTargetBoxWidget->getTargetBox();
}

void BoxItemWidgetContainer::drawKeys(QPainter *p, qreal pixelsPerFrame,
                                      int containerTop, int maxY,
                                      int minViewedFrame, int maxViewedFrame) {
    mTargetBoxWidget->drawKeys(p, pixelsPerFrame,
                               containerTop, maxY,
                               minViewedFrame, maxViewedFrame);
    if(mDetailsWidget->isVisible()) {
        foreach(ComplexAnimatorItemWidgetContainer *animatorContainer, mAnimatorsContainers) {
            int animatorTop = containerTop + BoxesListWidget::getListItemHeight() + animatorContainer->y();
            int animatorBottom = animatorTop + animatorContainer->height();
            if(animatorTop > maxY || animatorBottom < 0) continue;
            animatorContainer->drawKeys(p, pixelsPerFrame,
                                        animatorTop, maxY,
                                        minViewedFrame, maxViewedFrame);
        }
    }
}

QrealKey *BoxItemWidgetContainer::getKeyAtPos(qreal pressX, qreal pressY,
                                              qreal pixelsPerFrame, int maxY,
                                              int containerTop, int minViewedFrame) {
    if(pressY - containerTop <= BoxesListWidget::getListItemHeight()) {
        return mTargetBoxWidget->getTargetBox()->getAnimatorsCollection()->
                        getKeyAtPos(pressX, minViewedFrame, pixelsPerFrame);
    }
    if(mDetailsWidget->isVisible()) {
        foreach(ComplexAnimatorItemWidgetContainer *animatorContainer, mAnimatorsContainers) {
            int animatorTop = containerTop + BoxesListWidget::getListItemHeight() + animatorContainer->y();
            int animatorBottom = animatorTop + animatorContainer->height();
            if(animatorTop > maxY || animatorBottom < 0) continue;
            if(pressY > animatorTop && pressY < animatorBottom) {
                return animatorContainer->getKeyAtPos(pressX, pressY,
                                               pixelsPerFrame, maxY,
                                               animatorTop, minViewedFrame);
            }
        }
    }
    return NULL;
}

void BoxItemWidgetContainer::getKeysInRect(QRectF selectionRect, int containerTop,
                                           qreal pixelsPerFrame, int minViewedFrame,
                                           QList<QrealKey*> *listKeys) {
    if(selectionRect.top() < containerTop &&
       selectionRect.bottom() > containerTop + BoxesListWidget::getListItemHeight()) {
        mTargetBoxWidget->getTargetBox()->getAnimatorsCollection()->
                getKeysInRect(selectionRect,
                              minViewedFrame, pixelsPerFrame,
                              listKeys);
    }
    if(mDetailsWidget->isVisible()) {
        foreach(ComplexAnimatorItemWidgetContainer *animatorContainer, mAnimatorsContainers) {
            int animatorTop = containerTop + BoxesListWidget::getListItemHeight() + animatorContainer->y();
            int animatorBottom = animatorTop + animatorContainer->height();
            if(animatorTop > selectionRect.bottom() || animatorBottom < selectionRect.top()) continue;
            animatorContainer->getKeysInRect(selectionRect, animatorTop,
                                             pixelsPerFrame, minViewedFrame,
                                             listKeys);
        }
    }
}


void BoxItemWidgetContainer::addAnimatorWidgetForAnimator(QrealAnimator *animator)
{
    addAnimatorWidget(new ComplexAnimatorItemWidgetContainer(animator, this));
}

void BoxItemWidgetContainer::removeAnimatorWidgetForAnimator(QrealAnimator *animator)
{
    foreach(ComplexAnimatorItemWidgetContainer *animatorWidget, mAnimatorsContainers) {
        if(animatorWidget->getTargetAnimator() == animator) {
            removeAnimatorWidget(animatorWidget);
            break;
        }
    }
}

void BoxItemWidgetContainer::addAnimatorWidget(ComplexAnimatorItemWidgetContainer *widget)
{
    mChildWidgetsLayout->addWidget(widget);
    mAnimatorsContainers << widget;
}

void BoxItemWidgetContainer::removeAnimatorWidget(ComplexAnimatorItemWidgetContainer *widget)
{
    mAnimatorsContainers.removeOne(widget);
    delete widget;
}
