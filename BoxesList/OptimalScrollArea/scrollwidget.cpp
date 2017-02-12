#include "scrollwidget.h"
#include "scrollwidgetvisiblepart.h"
#include "singlewidget.h"
#include "singlewidgetabstraction.h"
#include "singlewidgettarget.h"

ScrollWidget::ScrollWidget(QWidget *parent) :
    QWidget(parent) {
    //createVisiblePartWidget();
}

void ScrollWidget::updateAbstraction() {
    if(mMainAbstraction != NULL) {
        mMainAbstraction->setIsMainTarget(false);
        if(mMainAbstraction->isDeletable()) {
            delete mMainAbstraction;
        }
    }
    if(mMainTarget == NULL) {
        mMainAbstraction = NULL;
    } else {
        mMainAbstraction->setIsMainTarget(true);
        mMainAbstraction = mMainTarget->SWT_getAbstractionForWidget(
                    mVisiblePartWidget);
    }
    mVisiblePartWidget->setMainAbstraction(mMainAbstraction);
    updateHeight();
}

void ScrollWidget::setWidth(const int &width) {
    setFixedWidth(width);
    mVisiblePartWidget->setFixedWidth(width);
    mVisiblePartWidget->updateWidgetsWidth();
}

void ScrollWidget::changeVisibleTop(const int &top) {
    mVisiblePartWidget->move(0, top);
    mVisiblePartWidget->setVisibleTop(top);
}

void ScrollWidget::changeVisibleHeight(const int &height) {
    mVisiblePartWidget->setFixedHeight(height);
    mVisiblePartWidget->setVisibleHeight(height);
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
                       false) + 10);
    if(isHidden()) show();
}
