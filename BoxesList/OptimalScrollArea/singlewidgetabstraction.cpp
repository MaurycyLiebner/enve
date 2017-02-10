#include "singlewidgetabstraction.h"
#include "singlewidgettarget.h"
#include "singlewidget.h"
#include "scrollwidgetvisiblepart.h"

SingleWidgetAbstraction::SingleWidgetAbstraction(
        SingleWidgetTarget *target,
        ScrollWidgetVisiblePart *visiblePart) {
    mTarget = target;
    mVisiblePartWidget = visiblePart;
}

SingleWidgetAbstraction::~SingleWidgetAbstraction() {
    mTarget->SWT_removeAbstractionFromList(this);
    foreach(SingleWidgetAbstraction *abs, mChildren) {
        delete abs;
    }
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
            SingleWidget *currWidget = widgets->at(*currentWidgetId);
            int currWx = currWidget->x();
            currWidget->move(currX, currWidget->y());
            currWidget->setFixedWidth(currWx - currX + currWidget->width());
            currWidget->setTargetAbstraction(this);
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

    if(mContentVisible) {
        mVisiblePartWidget->scheduledUpdateVisibleWidgetsContent();
        mVisiblePartWidget->scheduleUpdateParentHeight();
    }
}

void SingleWidgetAbstraction::removeChildAbstractionForTarget(
        SingleWidgetTarget *target) {
    SingleWidgetAbstraction *abstraction;
    foreach(SingleWidgetAbstraction *abs, mChildren) {
        if(abs->getTarget() == target) {
            abstraction = abs;
        }
    }
    mChildren.removeOne(abstraction);
    delete abstraction;

    if(mContentVisible) {
        mVisiblePartWidget->scheduledUpdateVisibleWidgetsContent();
        mVisiblePartWidget->scheduleUpdateParentHeight();
    }
}

void SingleWidgetAbstraction::switchContentVisible() {
    setContentVisible(!mContentVisible);
}

bool SingleWidgetAbstraction::contentVisible() {
    return mContentVisible;
}

void SingleWidgetAbstraction::setContentVisible(const bool &bT) {
    //if(bT == mContentVisible) return;
    mContentVisible = bT;
    mVisiblePartWidget->scheduledUpdateVisibleWidgetsContent();
    mVisiblePartWidget->scheduleUpdateParentHeight();
}

void SingleWidgetAbstraction::addChildAbstractionForTarget(
        SingleWidgetTarget *target) {
    addChildAbstraction(target->SWT_createAbstraction(mVisiblePartWidget));
}
