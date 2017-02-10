#include "scrollwidgetvisiblepart.h"
#include <QPainter>
#include "singlewidgetabstraction.h"
#include "singlewidget.h"
#include "scrollwidget.h"

QList<ScrollWidgetVisiblePart*> ScrollWidgetVisiblePart::mAllInstances;

ScrollWidgetVisiblePart::ScrollWidgetVisiblePart(
        ScrollWidget *parent) :
    QWidget(parent) {
    mParentWidget = parent;
    addInstance(this);
}

ScrollWidgetVisiblePart::~ScrollWidgetVisiblePart() {
    removeInstance(this);
}

void ScrollWidgetVisiblePart::setVisibleTop(
        const int &top) {
    mVisibleTop = top - top % 20;
    updateVisibleWidgetsContent();
}

void ScrollWidgetVisiblePart::setVisibleHeight(
        const int &height) {
    mVisibleHeight = height;
    updateVisibleWidgets();
}

void ScrollWidgetVisiblePart::updateWidgetsWidth() {
    foreach(SingleWidget *widget, mSingleWidgets) {
        widget->setFixedWidth(width() - widget->x());
    }
}

void ScrollWidgetVisiblePart::callUpdaters() {
    if(mVisibleWidgetsContentUpdateScheduled ||
       mParentHeightUpdateScheduled) {
        updateParentHeightIfNeeded();
        updateVisibleWidgetsContentIfNeeded();
    } else {
        update();
    }
}

void ScrollWidgetVisiblePart::callAllInstanceUpdaters() {
    foreach(ScrollWidgetVisiblePart *instance, mAllInstances) {
        instance->callUpdaters();
    }
}

void ScrollWidgetVisiblePart::addInstance(
        ScrollWidgetVisiblePart *instance) {
    mAllInstances.append(instance);
}

void ScrollWidgetVisiblePart::removeInstance(
        ScrollWidgetVisiblePart *instance) {
    mAllInstances.removeOne(instance);
}

void ScrollWidgetVisiblePart::scheduledUpdateVisibleWidgetsContent() {
    mVisibleWidgetsContentUpdateScheduled = true;
}

void ScrollWidgetVisiblePart::updateVisibleWidgetsContentIfNeeded() {
    if(mVisibleWidgetsContentUpdateScheduled) {
        mVisibleWidgetsContentUpdateScheduled = false;
        updateVisibleWidgetsContent();
    }
}

void ScrollWidgetVisiblePart::scheduleUpdateParentHeight() {
    mParentHeightUpdateScheduled = true;
}

void ScrollWidgetVisiblePart::updateParentHeightIfNeeded() {
    if(mParentHeightUpdateScheduled) {
        mParentHeightUpdateScheduled = false;
        updateParentHeight();
    }
}

void ScrollWidgetVisiblePart::updateVisibleWidgets() {
    int neededWidgets = ceil(mVisibleHeight/20.);
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
    foreach(SingleWidget *widget, mSingleWidgets) {
        widget->move(widget->x(), yT);
        widget->setFixedWidth(width() - widget->x());
        yT += 20;
    }

    updateVisibleWidgetsContent();
}

void ScrollWidgetVisiblePart::updateVisibleWidgetsContent() {
    if(mMainAbstraction == NULL) return;
    int idP = 0;
    mMainAbstraction->setSingleWidgetAbstractions(mVisibleTop,
                                                  mVisibleTop + mVisibleHeight + 10,
                                                  -10, -20,
                                                  &mSingleWidgets,
                                                  &idP);

    for(int i = idP; i < mSingleWidgets.count(); i++) {
        mSingleWidgets.at(i)->hide();
    }
}

void ScrollWidgetVisiblePart::setMainAbstraction(
        SingleWidgetAbstraction *abs) {
    mMainAbstraction = abs;
    if(abs == NULL) return;
    abs->setContentVisible(true);
//    updateVisibleWidgetsContent();
//    updateParentHeight();
}

void ScrollWidgetVisiblePart::updateParentHeight() {
    mParentWidget->updateHeight();
}

SingleWidget *ScrollWidgetVisiblePart::createNewSingleWidget() {
    return new SingleWidget(this);
}
