#include "singlewidgetabstraction.h"
#include "singlewidgettarget.h"
#include "singlewidget.h"
#include "scrollwidgetvisiblepart.h"
#include "global.h"

SingleWidgetAbstraction::SingleWidgetAbstraction(
        SingleWidgetTarget *target,
        ScrollWidgetVisiblePart *visiblePart) {
    mTarget = target;
    mVisiblePartWidget = visiblePart;
}

SingleWidgetAbstraction::~SingleWidgetAbstraction() {
    if(mTarget != NULL) {
        mTarget->SWT_removeAbstractionFromList(this);
    }
//    Q_FOREACH(SingleWidgetAbstraction *abs, mChildren) {
//        delete abs;
//    }
}

void SingleWidgetAbstraction::deleteWithDescendantAbstraction() {
    Q_FOREACH(SingleWidgetAbstraction *abs, mChildren) {
        abs->deleteWithDescendantAbstraction();
    }

    delete this;
}

bool SingleWidgetAbstraction::getAbstractions(
        const int &minY, const int &maxY,
        int *currY, int currX,
        QList<SingleWidgetAbstraction*> *abstractions,
        const SWT_RulesCollection &rules,
        const bool &parentSatisfiesRule,
        const bool &parentMainTarget) { // returns whether should abort
    if(*currY > maxY) return true;
    bool satisfiesRule = mTarget->SWT_shouldBeVisible(rules,
                                                      parentSatisfiesRule,
                                                      parentMainTarget);
    if(*currY > minY && satisfiesRule && !mIsMainTarget) {
        abstractions->append(this);
    }
    if(satisfiesRule && !mIsMainTarget) {
        currX += MIN_WIDGET_HEIGHT;
        *currY += MIN_WIDGET_HEIGHT;
    }
    Q_FOREACH(SingleWidgetAbstraction *abs, mChildren) {
        if(abs->getAbstractions(
                    minY, maxY,
                    currY, currX,
                    abstractions,
                    rules,
                    (satisfiesRule && mContentVisible) || mIsMainTarget,
                    mIsMainTarget)) {
            return true;
        }
    }

    return false;
}


bool SingleWidgetAbstraction::setSingleWidgetAbstractions(
        const int &minY, const int &maxY,
        int *currY, int currX,
        QList<SingleWidget *> *widgets,
        int *currentWidgetId,
        const SWT_RulesCollection &rules,
        const bool &parentSatisfiesRule,
        const bool &parentMainTarget) { // returns whether should abort
    if(*currY > maxY) return true;
    bool satisfiesRule = mTarget->SWT_shouldBeVisible(rules,
                                                      parentSatisfiesRule,
                                                      parentMainTarget);
    if(*currY > minY && satisfiesRule && !mIsMainTarget) {
        if(*currentWidgetId < widgets->count()) {
            SingleWidget *currWidget = widgets->at(*currentWidgetId);
            int currWx = currWidget->x();
            currWidget->move(currX, currWidget->y());
            currWidget->setFixedWidth(currWx - currX + currWidget->width());
            currWidget->setTargetAbstraction(this);
            *currentWidgetId = *currentWidgetId + 1;
        }
    }
    if(satisfiesRule && !mIsMainTarget) {
        currX += MIN_WIDGET_HEIGHT;
        *currY += MIN_WIDGET_HEIGHT;
    }
    Q_FOREACH(SingleWidgetAbstraction *abs, mChildren) {
        if(abs->setSingleWidgetAbstractions(
                    minY, maxY,
                    currY, currX,
                    widgets,
                    currentWidgetId,
                    rules,
                    (satisfiesRule && mContentVisible) || mIsMainTarget,
                    mIsMainTarget) ) {
            return true;
        }
    }

    return false;
}

int SingleWidgetAbstraction::getHeight(
        const SWT_RulesCollection &rules,
        const bool &parentSatisfiesRule,
        const bool &parentMainTarget) {
    int height = 0;
    bool satisfiesRule = mTarget->SWT_shouldBeVisible(rules,
                                                      parentSatisfiesRule,
                                                      parentMainTarget);
    if(satisfiesRule && !mIsMainTarget) {
        height += MIN_WIDGET_HEIGHT;
    }
    Q_FOREACH(SingleWidgetAbstraction *abs, mChildren) {
        height += abs->getHeight(
                    rules,
                    (satisfiesRule && mContentVisible) || mIsMainTarget,
                    mIsMainTarget);
    }

    return height;
}

void SingleWidgetAbstraction::addChildAbstraction(
        SingleWidgetAbstraction *abs) {
    mChildren.append(abs);

    mVisiblePartWidget->scheduledUpdateVisibleWidgetsContent();
    mVisiblePartWidget->scheduleUpdateParentHeight();
}

void SingleWidgetAbstraction::addChildAbstractionAt(
        SingleWidgetAbstraction *abs, const int &id) {
    mChildren.insert(id, abs);

    mVisiblePartWidget->scheduledUpdateVisibleWidgetsContent();
    mVisiblePartWidget->scheduleUpdateParentHeight();
}

SingleWidgetAbstraction *SingleWidgetAbstraction::getChildAbstractionForTarget(
        SingleWidgetTarget *target) {
    Q_FOREACH(SingleWidgetAbstraction *abs, mChildren) {
        if(abs->getTarget() == target) {
            return abs;
        }
    }
    return NULL;
}

void SingleWidgetAbstraction::removeChildAbstractionForTarget(
        SingleWidgetTarget *target) {
    removeChildAbstraction(getChildAbstractionForTarget(target));
}

void SingleWidgetAbstraction::removeChildAbstraction(
        SingleWidgetAbstraction *abs) {
    mChildren.removeOne(abs);
    delete abs;

    mVisiblePartWidget->scheduledUpdateVisibleWidgetsContent();
    mVisiblePartWidget->scheduleUpdateParentHeight();
}

void SingleWidgetAbstraction::switchContentVisible() {
    setContentVisible(!mContentVisible);
}

bool SingleWidgetAbstraction::contentVisible() {
    return mContentVisible;
}

void SingleWidgetAbstraction::scheduleWidgetContentUpdateIfIsCurrentRule(
        const SWT_Rule &rule) {
    mVisiblePartWidget->scheduleContentUpdateIfIsCurrentRule(rule);
}

void SingleWidgetAbstraction::scheduleWidgetContentUpdateIfIsCurrentTarget(
        SingleWidgetTarget *targetP,
        const SWT_Target &target) {
    mVisiblePartWidget->scheduleContentUpdateIfIsCurrentTarget(targetP,
                                                               target);
}

void SingleWidgetAbstraction::scheduleWidgetContentUpdateIfSearchNotEmpty() {
    mVisiblePartWidget->scheduleContentUpdateIfSearchNotEmpty();
}

void SingleWidgetAbstraction::setContentVisible(const bool &bT) {
    //if(bT == mContentVisible) return;
    mContentVisible = bT;
    mVisiblePartWidget->updateParentHeight();
    mVisiblePartWidget->updateVisibleWidgetsContent();
}

void SingleWidgetAbstraction::addChildAbstractionForTarget(
        SingleWidgetTarget *target) {
    addChildAbstraction(
                target->SWT_getAbstractionForWidget(mVisiblePartWidget));
}

void SingleWidgetAbstraction::addChildAbstractionForTargetAt(
        SingleWidgetTarget *target, const int &id) {
    addChildAbstractionAt(
                target->SWT_getAbstractionForWidget(mVisiblePartWidget),
                id);

}

void SingleWidgetAbstraction::moveChildAbstractionForTargetTo(
        SingleWidgetTarget *target, const int &id) {
    SingleWidgetAbstraction *abs = getChildAbstractionForTarget(target);
    int targetId = id;
//    if(!abs->getTarget()->SWT_visibleOnlyIfParentDescendant()) {
//        for(int i = 0; i < mChildren.count(); i++) {
//            SingleWidgetAbstraction *abs1 = mChildren.at(i);
//            if(abs1->getTarget()->SWT_visibleOnlyIfParentDescendant()) {
//                targetId++;
//            } else {
//                break;
//            }
//        }
//    }
    int currId = mChildren.indexOf(abs);
    mChildren.move(currId, targetId);

    mVisiblePartWidget->scheduledUpdateVisibleWidgetsContent();
    mVisiblePartWidget->scheduleUpdateParentHeight();
}
