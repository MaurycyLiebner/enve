#include "scrollwidgetvisiblepart.h"
#include <QPainter>
#include "singlewidgetabstraction.h"
#include "singlewidget.h"
#include "scrollwidget.h"
#include "singlewidgettarget.h"
#include "GUI/mainwindow.h"
#include "global.h"

ScrollWidgetVisiblePart::ScrollWidgetVisiblePart(
        ScrollWidget *parent) :
    MinimalScrollWidgetVisiblePart(parent) {
    mCurrentRulesCollection.rule = SWT_NoRule;
}

ScrollWidgetVisiblePart::~ScrollWidgetVisiblePart() {
    removeInstance(this);
}

void ScrollWidgetVisiblePart::callUpdaters() {
    if(mVisibleWidgetsContentUpdateScheduled ||
       mParentHeightUpdateScheduled) {
        updateParentHeightIfNeeded();
        updateVisibleWidgetsContentIfNeeded();
    }
    update();
}

void ScrollWidgetVisiblePart::setCurrentRule(
        const SWT_Rule &rule) {
    mCurrentRulesCollection.rule = rule;
    updateParentHeight();
    updateVisibleWidgetsContent();
}

void ScrollWidgetVisiblePart::setCurrentTarget(
        SingleWidgetTarget* targetP,
        const SWT_Target &target) {
    mCurrentRulesCollection.target = target;
    static_cast<ScrollWidget*>(mParentWidget)->setMainTarget(targetP);
    updateParentHeight();
    updateVisibleWidgetsContent();
}

void ScrollWidgetVisiblePart::setCurrentType(SWT_Checker type) {
    mCurrentRulesCollection.type = type;
    updateParentHeight();
    updateVisibleWidgetsContent();
}

void ScrollWidgetVisiblePart::setAlwaysShowChildren(
        const bool &alwaysShowChildren) {
    mCurrentRulesCollection.alwaysShowChildren = alwaysShowChildren;
    updateParentHeight();
    updateVisibleWidgetsContent();
}

void ScrollWidgetVisiblePart::setCurrentSearchText(
        const QString &text) {
    mCurrentRulesCollection.searchString = text;
    updateParentHeight();
    updateVisibleWidgetsContent();
}

void ScrollWidgetVisiblePart::scheduleContentUpdateIfIsCurrentRule(
        const SWT_Rule &rule) {
    if(isCurrentRule(rule)) {
        scheduleUpdateParentHeight();
        scheduledUpdateVisibleWidgetsContent();
    }
}

void ScrollWidgetVisiblePart::scheduleContentUpdateIfIsCurrentTarget(
        SingleWidgetTarget* targetP,
        const SWT_Target &target) {
    if(mCurrentRulesCollection.target == target) {
        static_cast<ScrollWidget*>(mParentWidget)->setMainTarget(targetP);
        scheduleUpdateParentHeight();
        scheduledUpdateVisibleWidgetsContent();
    }
}

void ScrollWidgetVisiblePart::scheduleContentUpdateIfSearchNotEmpty() {
    if(mCurrentRulesCollection.searchString.isEmpty()) return;
    scheduleUpdateParentHeight();
    scheduledUpdateVisibleWidgetsContent();
}

bool ScrollWidgetVisiblePart::isCurrentRule(const SWT_Rule &rule) {
    return rule == mCurrentRulesCollection.rule;
}

void ScrollWidgetVisiblePart::updateVisibleWidgetsContent() {
    if(mMainAbstraction == nullptr) return;
    //updateParentHeight();
    int idP = 0;
    int currX;
    int currY;
//    if(mSkipMainAbstraction) {
//        currX = -20;
//        currY = -10;
//    } else {
        currX = 0;
        currY = MIN_WIDGET_HEIGHT/2;
//    }
    mMainAbstraction->setSingleWidgetAbstractions(
                mVisibleTop,
                mVisibleTop + mVisibleHeight + currY,
                &currY, currX,
                &mSingleWidgets,
                &idP,
                mCurrentRulesCollection,
                true,
                false);

    for(int i = idP; i < mSingleWidgets.count(); i++) {
        mSingleWidgets.at(i)->hide();
    }
}

void ScrollWidgetVisiblePart::setMainAbstraction(
        SingleWidgetAbstraction* abs) {
    mMainAbstraction = abs;
    scheduledUpdateVisibleWidgetsContent();
//    if(abs == nullptr) return;
//    abs->setContentVisible(true);
//    updateVisibleWidgetsContent();
//    updateParentHeight();
}

QWidget *ScrollWidgetVisiblePart::createNewSingleWidget() {
    return new SingleWidget(this);
}

SWT_RulesCollection::SWT_RulesCollection() {
    type = nullptr;
    rule = SWT_NoRule;
    alwaysShowChildren = false;
    target = SWT_CurrentCanvas;
    searchString = "";
}
