#include "singlewidgetabstraction.h"
#include "singlewidgettarget.h"
#include "scrollwidgetvisiblepart.h"
#include "singlewidget.h"

SingleWidgetAbstraction::SingleWidgetAbstraction(
        SingleWidgetTarget *target,
        ScrollWidgetVisiblePart *visiblePartWidget) {
    mTarget = target;
    mVisiblePartWidget = visiblePartWidget;
}

bool SingleWidgetAbstraction::setSingleWidgetAbstractions(
        const int &minY, const int &maxY,
        int currY, int currX,
        QList<SingleWidget *> *widgets,
        int *currentWidgetId) { // returns whether should abort
    int thisHeight = getHeight();
    if(currY + thisHeight < minY) return false;
    if(currY > maxY) return true;
    if(currY > minY) {
        if(*currentWidgetId < widgets->count()) {
            widgets->at(*currentWidgetId)->setTargetAbstraction(this);
            *currentWidgetId = *currentWidgetId + 1;
        }
    }
    currX += 20;
    if(mContentVisible) {
        currY += 20;
        foreach(SingleWidgetAbstraction *abs, mChildren) {
            if(abs->setSingleWidgetAbstractions(minY, maxY,
                                                currY, currX,
                                                widgets,
                                                currentWidgetId) ) {
                return true;
            }
            currY += abs->getHeight();
        }
    }

    return false;
}

int SingleWidgetAbstraction::getHeight() {
    if(mContentVisible) {
        int height = 20;
        foreach(SingleWidgetAbstraction *abs, mChildren) {
            height += abs->getHeight();
        }
        return height;
    } else {
        return 20;
    }
}

void SingleWidgetAbstraction::addChildAbstraction(
        SingleWidgetAbstraction *abs) {
    mChildren.append(abs);
}

void SingleWidgetAbstraction::setContentVisible(const bool &bT) {
    //if(bT == mContentVisible) return;
    mContentVisible = bT;
    mVisiblePartWidget->updateVisibleWidgetsContent();
    mVisiblePartWidget->updateParentHeight();
}
