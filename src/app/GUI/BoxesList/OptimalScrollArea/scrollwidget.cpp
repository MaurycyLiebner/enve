#include "scrollwidget.h"
#include "scrollwidgetvisiblepart.h"
#include "singlewidget.h"
#include "singlewidgetabstraction.h"
#include "singlewidgettarget.h"
#include "scrollarea.h"
#include "GUI/global.h"

ScrollWidget::ScrollWidget(ScrollArea * const parent) :
    MinimalScrollWidget(parent) {
    //createVisiblePartWidget();
    connect(parent, &ScrollArea::heightChanged,
            this, &ScrollWidget::updateHeightAfterScrollAreaResize);
}

void ScrollWidget::updateAbstraction() {
    if(mMainAbstraction) {
        mMainAbstraction->setIsMainTarget(false);
//        if(mMainAbstraction->isDeletable()) {
//            delete mMainAbstraction;
//        }
    }
    if(!mMainTarget) {
        mMainAbstraction = nullptr;
    } else {
        int widId = mVisiblePartWidget->getId();
        mMainAbstraction = mMainTarget->SWT_getAbstractionForWidget(widId);
        mMainAbstraction->setIsMainTarget(true);
    }
    mVisiblePartWidget->setMainAbstraction(mMainAbstraction);
    updateHeight();
}

void ScrollWidget::createVisiblePartWidget() {
    mVisiblePartWidget = new ScrollWidgetVisiblePart(this);
    mMinimalVisiblePartWidget = mVisiblePartWidget;
}

void ScrollWidget::setMainTarget(SingleWidgetTarget *target) {
    mMainTarget = target;
    updateAbstraction();
}

void ScrollWidget::updateHeight() {
    if(!mMainAbstraction) return hide();
    mContentHeight = mMainAbstraction->updateHeight(
                mVisiblePartWidget->getCurrentRulesCollection(),
                false, false, MIN_WIDGET_DIM) + MIN_WIDGET_DIM/2;
    const int parentHeight = mParentScrollArea->height();
    setFixedHeight(qMax(mContentHeight, parentHeight));
    if(isHidden()) show();
}

void ScrollWidget::updateHeightAfterScrollAreaResize(const int parentHeight) {
    setFixedHeight(qMax(mContentHeight, parentHeight));
}
