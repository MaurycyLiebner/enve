#include "scrollwidgetvisiblepart.h"
#include <QPainter>
#include "singlewidgetabstraction.h"
#include "singlewidget.h"
#include "scrollwidget.h"

ScrollWidgetVisiblePart::ScrollWidgetVisiblePart(
        ScrollWidget *parent) :
    QWidget(parent) {
    mParentWidget = parent;
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
        widget->move(0, yT);
        widget->setFixedWidth(100);
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
    if(mSingleWidgets.count() > 0) {
        mSingleWidgets.at(0)->setStyleSheet("background-color: green;");
    }
    for(int i = idP; i < mSingleWidgets.count(); i++) {
        mSingleWidgets.at(i)->hide();
    }
}

void ScrollWidgetVisiblePart::setMainAbstraction(
        SingleWidgetAbstraction *abs) {
    mMainAbstraction = abs;
    if(abs == NULL) return;
    abs->setContentVisible(true);
}

void ScrollWidgetVisiblePart::updateParentHeight() {
    mParentWidget->updateHeight();
}

SingleWidget *ScrollWidgetVisiblePart::createNewSingleWidget() {
    return new SingleWidget(this);
}
