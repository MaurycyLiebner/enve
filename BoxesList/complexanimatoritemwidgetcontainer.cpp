#include "complexanimatoritemwidgetcontainer.h"
#include "qrealanimatoritemwidget.h"
#include "Animators/qrealanimator.h"
#include "complexanimatoritemwidget.h"
#include "Animators/complexanimator.h"
#include "boxeslistwidget.h"
#include "effectanimatorsitemwidgetcontainer.h"

ComplexAnimatorItemWidgetContainer::ComplexAnimatorItemWidgetContainer(QrealAnimator *target,
                                                                       QWidget *parent) :
    WidgetContainer(parent)
{
    setAcceptDrops(true);
    initialize();
    if(target->isComplexAnimator()) {
        mTargetAnimatorWidget = new ComplexAnimatorItemWidget(target, this);

        connect((ComplexAnimatorItemWidget*)mTargetAnimatorWidget, SIGNAL(detailsVisibilityChanged(bool)),
                mDetailsWidget, SLOT(setVisible(bool)));
        ((ComplexAnimator*)target)->addAllAnimatorsToComplexAnimatorItemWidgetContainer(this);

        connect(target, SIGNAL(childAnimatorAdded(QrealAnimator*)),
                this, SLOT(addChildAnimator(QrealAnimator*)));
        connect(target, SIGNAL(childAnimatorRemoved(QrealAnimator*)),
                this, SLOT(removeChildAnimator(QrealAnimator*)));
        connect(target, SIGNAL(childAnimatorZChanged(int,int)),
                this, SLOT(changeChildAnimatorZ(int,int)));
    } else {
        mTargetAnimatorWidget = new QrealAnimatorItemWidget(target, this);
    }

    setTopWidget(mTargetAnimatorWidget);
}

QrealAnimator *ComplexAnimatorItemWidgetContainer::getTargetAnimator() {
    return mTargetAnimatorWidget->getTargetAnimator();
}

void ComplexAnimatorItemWidgetContainer::addChildAnimator(QrealAnimator *animator)
{
    ComplexAnimatorItemWidgetContainer *itemWidget;
    if(animator->isEffectsAnimator()) {
        itemWidget =
            new EffectAnimatorsItemWidgetContainer(animator, this);
    } else {
        itemWidget =
            new ComplexAnimatorItemWidgetContainer(animator, this);
    }

    addChildWidget(itemWidget);

    mChildWidgets << itemWidget;
}

void ComplexAnimatorItemWidgetContainer::removeChildAnimator(
        QrealAnimator *animator) {
    foreach(ComplexAnimatorItemWidgetContainer *child, mChildWidgets) {
        if(child->getTargetAnimator() == animator) {
            mChildWidgets.removeOne(child);
            delete child;
            break;
        }
    }
}

void ComplexAnimatorItemWidgetContainer::changeChildAnimatorZ(const int &from,
                                                              const int &to) {
    mChildWidgetsLayout->insertWidget(to,
                                mChildWidgetsLayout->itemAt(from)->widget());
}

void ComplexAnimatorItemWidgetContainer::drawKeys(QPainter *p, qreal pixelsPerFrame,
                                                  int animatorTop, int maxY,
                                                  int minViewedFrame, int maxViewedFrame) {
    mTargetAnimatorWidget->drawKeys(p, pixelsPerFrame,
                                    animatorTop, maxY,
                                    minViewedFrame, maxViewedFrame);
    if(mDetailsWidget->isVisible()) {
        foreach(ComplexAnimatorItemWidgetContainer *animatorContainer, mChildWidgets) {
            int childAnimatorTop = animatorTop + BoxesListWidget::getListItemHeight() + animatorContainer->y();
            int childAnimatorBottom = childAnimatorTop + animatorContainer->height();
            if(childAnimatorTop > maxY || childAnimatorBottom < 0) continue;
            animatorContainer->drawKeys(p, pixelsPerFrame,
                                        childAnimatorTop, maxY,
                                        minViewedFrame, maxViewedFrame);
        }
    }
}

QrealKey *ComplexAnimatorItemWidgetContainer::getKeyAtPos(qreal pressX, qreal pressY,
                                                          qreal pixelsPerFrame, int maxY,
                                                          int animatorTop, int minViewedFrame)
{
    if(pressY - animatorTop <= BoxesListWidget::getListItemHeight()) {
        return mTargetAnimatorWidget->getTargetAnimator()->
                        getKeyAtPos(pressX, minViewedFrame, pixelsPerFrame);
    }
    if(mDetailsWidget->isVisible()) {
        foreach(ComplexAnimatorItemWidgetContainer *animatorContainer, mChildWidgets) {
            int childAnimatorTop = animatorTop + BoxesListWidget::getListItemHeight() + animatorContainer->y();
            int childAnimatorBottom = childAnimatorTop + animatorContainer->height();
            if(childAnimatorTop > maxY || childAnimatorBottom < 0) continue;
            if(pressY > childAnimatorTop && pressY < childAnimatorBottom) {
                return animatorContainer->getKeyAtPos(pressX, pressY,
                                               pixelsPerFrame, maxY,
                                               childAnimatorTop, minViewedFrame);
            }
        }
    }
    return NULL;
}

void ComplexAnimatorItemWidgetContainer::
getKeysInRect(QRectF selectionRect, int containerTop,
              qreal pixelsPerFrame, int minViewedFrame,
              QList<QrealKey*> *listKeys) {
    if(selectionRect.top() < containerTop &&
       selectionRect.bottom() > containerTop + BoxesListWidget::getListItemHeight()) {
        mTargetAnimatorWidget->getTargetAnimator()->
                        getKeysInRect(selectionRect, minViewedFrame,
                                      pixelsPerFrame, listKeys);
    }
    if(mDetailsWidget->isVisible()) {
        foreach(ComplexAnimatorItemWidgetContainer *animatorContainer, mChildWidgets) {
            int childAnimatorTop = containerTop + BoxesListWidget::getListItemHeight() + animatorContainer->y();
            int childAnimatorBottom = childAnimatorTop + animatorContainer->height();
            if(childAnimatorTop > selectionRect.bottom() || childAnimatorBottom < selectionRect.top()) continue;
            animatorContainer->getKeysInRect(selectionRect, childAnimatorTop,
                                             pixelsPerFrame, minViewedFrame,
                                             listKeys);
        }
    }
}
