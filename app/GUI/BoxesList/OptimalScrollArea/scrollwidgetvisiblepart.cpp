#include "scrollwidgetvisiblepart.h"
#include <QPainter>
#include "singlewidgetabstraction.h"
#include "singlewidget.h"
#include "scrollwidget.h"
#include "singlewidgettarget.h"
#include "GUI/mainwindow.h"
#include "global.h"

int ScrollWidgetVisiblePart::mNextId = 0;

ScrollWidgetVisiblePart::ScrollWidgetVisiblePart(
        ScrollWidget *parent) :
    MinimalScrollWidgetVisiblePart(parent), mId(mNextId++) {
    mCurrentRulesCollection.rule = SWT_NoRule;
    setupUpdateFuncs();
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
        scheduleUpdateVisibleWidgetsContent();
    }
}

void ScrollWidgetVisiblePart::scheduleContentUpdateIfIsCurrentTarget(
        SingleWidgetTarget* targetP,
        const SWT_Target &target) {
    if(mCurrentRulesCollection.target == target) {
        static_cast<ScrollWidget*>(mParentWidget)->setMainTarget(targetP);
        scheduleUpdateParentHeight();
        scheduleUpdateVisibleWidgetsContent();
    }
}

void ScrollWidgetVisiblePart::scheduleContentUpdateIfSearchNotEmpty() {
    if(mCurrentRulesCollection.searchString.isEmpty()) return;
    scheduleUpdateParentHeight();
    scheduleUpdateVisibleWidgetsContent();
}

bool ScrollWidgetVisiblePart::isCurrentRule(const SWT_Rule &rule) {
    return rule == mCurrentRulesCollection.rule;
}

void ScrollWidgetVisiblePart::updateVisibleWidgetsContent() {
    if(!mMainAbstraction) return;
    //updateParentHeight();

    int currentWidgetId = 0;
    SetAbsFunc setAbsFunc = [this, &currentWidgetId](
            SingleWidgetAbstraction* newAbs,
            int currX) {
        if(currentWidgetId < mSingleWidgets.count()) {
            SingleWidget *currWidget =
                    static_cast<SingleWidget*>(
                        mSingleWidgets.at(currentWidgetId));
            currWidget->setTargetAbstraction(newAbs);
            int currWx = currWidget->x();
            currWidget->move(currX, currWidget->y());
            currWidget->setFixedWidth(currWx - currX + currWidget->width());
            currentWidgetId = currentWidgetId + 1;
        }
    };
    int currX = 0;
    int currY = MIN_WIDGET_HEIGHT/2;
    mMainAbstraction->setSingleWidgetAbstractions(
                mVisibleTop,
                mVisibleTop + mVisibleHeight + currY,
                currY, currX, MIN_WIDGET_HEIGHT,
                setAbsFunc,
                mCurrentRulesCollection,
                true, false);

    for(int i = currentWidgetId; i < mSingleWidgets.count(); i++) {
        mSingleWidgets.at(i)->hide();
    }
}

void ScrollWidgetVisiblePart::setMainAbstraction(
        SingleWidgetAbstraction* abs) {
    mMainAbstraction = abs;
    scheduleUpdateVisibleWidgetsContent();
//    if(!abs) return;
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
