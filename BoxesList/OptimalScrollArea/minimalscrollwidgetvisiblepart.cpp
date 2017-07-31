#include "minimalMinimalScrollWidgetVisiblePart.h"

#include "minimalscrollwidgetvisiblepart.h"
#include <QPainter>
#include "singlewidgetabstraction.h"
#include "singlewidget.h"
#include "scrollwidget.h"
#include "singlewidgettarget.h"
#include "mainwindow.h"
#include "global.h"

QList<MinimalScrollWidgetVisiblePart*> MinimalScrollWidgetVisiblePart::mAllInstances;

MinimalScrollWidgetVisiblePart::MinimalScrollWidgetVisiblePart(
        MinimalScrollWidget *parent) :
    QWidget(parent) {
    mCurrentRulesCollection.rule = SWT_NoRule;
    mParentWidget = parent;
    addInstance(this);
}

MinimalScrollWidgetVisiblePart::~MinimalScrollWidgetVisiblePart() {
    removeInstance(this);
}

void MinimalScrollWidgetVisiblePart::setVisibleTop(const int &top) {
    mVisibleTop = top;
    updateVisibleWidgetsContent();
}

void MinimalScrollWidgetVisiblePart::setVisibleHeight(const int &height) {
    setFixedHeight(height);
    mVisibleHeight = height;
    updateVisibleWidgets();
}

void MinimalScrollWidgetVisiblePart::updateWidgetsWidth() {
    Q_FOREACH(SingleWidget *widget, mSingleWidgets) {
        widget->setFixedWidth(width() - widget->x());
    }
}

void MinimalScrollWidgetVisiblePart::callUpdaters() {
    if(mVisibleWidgetsContentUpdateScheduled ||
       mParentHeightUpdateScheduled) {
        updateParentHeightIfNeeded();
        updateVisibleWidgetsContentIfNeeded();
    }
    update();
}

void MinimalScrollWidgetVisiblePart::callAllInstanceUpdaters() {
    Q_FOREACH(MinimalScrollWidgetVisiblePart *instance, mAllInstances) {
        instance->callUpdaters();
    }
}

void MinimalScrollWidgetVisiblePart::addInstance(
        MinimalScrollWidgetVisiblePart *instance) {
    mAllInstances.append(instance);
}

void MinimalScrollWidgetVisiblePart::removeInstance(
        MinimalScrollWidgetVisiblePart *instance) {
    mAllInstances.removeOne(instance);
}

void MinimalScrollWidgetVisiblePart::setAlwaysShowChildren(
        const bool &alwaysShowChildren) {
    mCurrentRulesCollection.alwaysShowChildren = alwaysShowChildren;
    updateParentHeight();
    updateVisibleWidgetsContent();
}

void MinimalScrollWidgetVisiblePart::setCurrentSearchText(
        const QString &text) {
    mCurrentRulesCollection.searchString = text;
    updateParentHeight();
    updateVisibleWidgetsContent();
}

void MinimalScrollWidgetVisiblePart::scheduleContentUpdate() {
    scheduleUpdateParentHeight();
    scheduledUpdateVisibleWidgetsContent();
}

void MinimalScrollWidgetVisiblePart::scheduleContentUpdateIfSearchNotEmpty() {
    if(mCurrentRulesCollection.searchString.isEmpty()) return;
    scheduleUpdateParentHeight();
    scheduledUpdateVisibleWidgetsContent();
}

bool MinimalScrollWidgetVisiblePart::isCurrentRule(const SWT_Rule &rule) {
    return rule == mCurrentRulesCollection.rule;
}

void MinimalScrollWidgetVisiblePart::scheduledUpdateVisibleWidgetsContent() {
    mVisibleWidgetsContentUpdateScheduled = true;
}

void MinimalScrollWidgetVisiblePart::updateVisibleWidgetsContentIfNeeded() {
    if(mVisibleWidgetsContentUpdateScheduled) {
        mVisibleWidgetsContentUpdateScheduled = false;
        updateVisibleWidgetsContent();
    }
}

void MinimalScrollWidgetVisiblePart::scheduleUpdateParentHeight() {
    mParentHeightUpdateScheduled = true;
}

void MinimalScrollWidgetVisiblePart::updateParentHeightIfNeeded() {
    if(mParentHeightUpdateScheduled) {
        mParentHeightUpdateScheduled = false;
        updateParentHeight();
    }
}

void MinimalScrollWidgetVisiblePart::updateVisibleWidgets() {
    int neededWidgets = qCeil(mVisibleHeight/
                              (qreal)MIN_WIDGET_HEIGHT);
    int currentNWidgets = mSingleWidgets.count();

    if(neededWidgets == currentNWidgets) return;
    if(neededWidgets > currentNWidgets) {
        for(int i = neededWidgets - currentNWidgets; i > 0; i--) {
            SingleWidget *newWidget = createNewSingleWidget();
            mSingleWidgets.append(newWidget);
        }
    } else {
        for(int i = currentNWidgets - neededWidgets; i > 0; i--) {
            delete mSingleWidgets.takeLast();
        }
    }

    int yT = 0;
    Q_FOREACH(SingleWidget *widget, mSingleWidgets) {
        widget->move(widget->x(), yT);
        widget->setFixedWidth(width() - widget->x());
        yT += MIN_WIDGET_HEIGHT;
    }

    updateVisibleWidgetsContent();
}

void MinimalScrollWidgetVisiblePart::updateVisibleWidgetsContent() {
    int idP = 0;
    int currX;
    int currY;
    currX = 0;
    currY = MIN_WIDGET_HEIGHT/2;
//    mMainAbstraction->setSingleWidgetAbstractions(
//                mVisibleTop,
//                mVisibleTop + mVisibleHeight + currY,
//                &currY, currX,
//                &mSingleWidgets,
//                &idP,
//                mCurrentRulesCollection,
//                true,
//                false);

    for(int i = idP; i < mSingleWidgets.count(); i++) {
        mSingleWidgets.at(i)->hide();
    }
}

void MinimalScrollWidgetVisiblePart::updateParentHeight() {
    mParentWidget->updateHeight();
}

SingleWidget *MinimalScrollWidgetVisiblePart::createNewSingleWidget() {
    return new SingleWidget(this);
}

SWT_RulesCollection::SWT_RulesCollection() {
    type = NULL;
    rule = SWT_NoRule;
    alwaysShowChildren = false;
    target = SWT_CurrentCanvas;
    searchString = "";
}
