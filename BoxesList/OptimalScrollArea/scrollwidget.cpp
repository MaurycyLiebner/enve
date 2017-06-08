#include "scrollwidget.h"
#include "scrollwidgetvisiblepart.h"
#include "singlewidget.h"
#include "singlewidgetabstraction.h"
#include "singlewidgettarget.h"
#include "scrollarea.h"
#include "global.h"

ScrollWidget::ScrollWidget(ScrollArea *parent) :
    QWidget(parent) {
    mParentScrollArea = parent;
    //createVisiblePartWidget();
}

void ScrollWidget::updateAbstraction() {
    if(mMainAbstraction != NULL) {
        mMainAbstraction->setIsMainTarget(false);
//        if(mMainAbstraction->isDeletable()) {
//            delete mMainAbstraction;
//        }
    }
    if(mMainTarget == NULL) {
        mMainAbstraction = NULL;
    } else {
        mMainAbstraction = mMainTarget->SWT_getAbstractionForWidget(
                                                mVisiblePartWidget);
        mMainAbstraction->setIsMainTarget(true);
    }
    mVisiblePartWidget->setMainAbstraction(mMainAbstraction);
    updateHeight();
}

void ScrollWidget::scrollParentAreaBy(const int &by) {
    mParentScrollArea->scrollBy(0, by);
}

void ScrollWidget::setWidth(const int &width) {
    setFixedWidth(width);
    mVisiblePartWidget->setFixedWidth(width);
    mVisiblePartWidget->updateWidgetsWidth();
}

void ScrollWidget::changeVisibleTop(const int &top) {
    int newTop = top - top % MIN_WIDGET_HEIGHT;
    mVisiblePartWidget->move(0, newTop);
    mVisiblePartWidget->setVisibleTop(newTop);
}
#include <QtMath>
void ScrollWidget::changeVisibleHeight(const int &height) {
    int newHeight = qCeil(height/(qreal)MIN_WIDGET_HEIGHT)*
                    MIN_WIDGET_HEIGHT;
    mVisiblePartWidget->setVisibleHeight(newHeight);
}

void ScrollWidget::createVisiblePartWidget() {
    mVisiblePartWidget = new ScrollWidgetVisiblePart(this);
}

void ScrollWidget::setMainTarget(SingleWidgetTarget *target) {
    mMainTarget = target;
    updateAbstraction();
}

void ScrollWidget::updateHeight() {
    if(mMainAbstraction == NULL) {
        hide();
        return;
    }
    setFixedHeight(mMainAbstraction->getHeight(
                       mVisiblePartWidget->getCurrentRulesCollection(),
                       false,
                       false) + MIN_WIDGET_HEIGHT/2);
    if(isHidden()) show();
}
