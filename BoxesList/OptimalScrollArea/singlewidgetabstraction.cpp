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
    if(mTarget != NULL) {
        mTarget->SWT_removeAbstractionFromList(this);
    }
    foreach(SingleWidgetAbstraction *abs, mChildren) {
        delete abs;
    }
}

bool SingleWidgetAbstraction::setSingleWidgetAbstractions(
        const int &minY, const int &maxY,
        int currY, int currX,
        QList<SingleWidget *> *widgets,
        int *currentWidgetId,
        const SWT_RulesCollection &rules,
        const bool &parentSatisfiesRule,
        const bool &targetDescendant) { // returns whether should abort
    int thisHeight = getHeight(rules,
                               parentSatisfiesRule,
                               targetDescendant);
    if(currY + thisHeight < minY) return false;
    if(currY > maxY) return true;
    bool satisfiesRule = mTarget->SWT_satisfiesRule(rules,
                                                    parentSatisfiesRule);
    if(currY > minY && satisfiesRule && targetDescendant) {
        if(*currentWidgetId < widgets->count()) {
            SingleWidget *currWidget = widgets->at(*currentWidgetId);
            int currWx = currWidget->x();
            currWidget->move(currX, currWidget->y());
            currWidget->setFixedWidth(currWx - currX + currWidget->width());
            currWidget->setTargetAbstraction(this);
            *currentWidgetId = *currentWidgetId + 1;
        }
    }
    if(mContentVisible || !targetDescendant) {
        bool childrenDescendants = targetDescendant ||
                isMainTarget(rules);
        if(satisfiesRule && targetDescendant) {
            currX += 20;
            currY += 20;
        }
        foreach(SingleWidgetAbstraction *abs, mChildren) {
            bool childDescendant =
                (abs->getTarget()->SWT_visibleOnlyIfParentDescendant()
                 ? targetDescendant
                 : childrenDescendants);
            if(abs->setSingleWidgetAbstractions(minY, maxY,
                                                currY, currX,
                                                widgets,
                                                currentWidgetId,
                                                rules,
                                                satisfiesRule,
                                                childDescendant) ) {
                return true;
            }
            currY += abs->getHeight(rules,
                                    satisfiesRule,
                                    childrenDescendants);
        }
    }

    return false;
}

int SingleWidgetAbstraction::getHeight(
        const SWT_RulesCollection &rules,
        const bool &parentSatisfiesRule,
        const bool &targetDescendant) {
    int height = 0;
    bool satisfiesRule = mTarget->SWT_satisfiesRule(rules,
                                                    parentSatisfiesRule);
    if(satisfiesRule && targetDescendant) {
        height += 20;
    }
    if(mContentVisible) {
        bool childrenDescendants = targetDescendant ||
                isMainTarget(rules);
        foreach(SingleWidgetAbstraction *abs, mChildren) {
            bool childDescendant =
                (abs->getTarget()->SWT_visibleOnlyIfParentDescendant()
                 ? targetDescendant
                 : childrenDescendants);
            height += abs->getHeight(rules, satisfiesRule,
                                     childDescendant);
        }
    }

    return height;
}

void SingleWidgetAbstraction::addChildAbstraction(
        SingleWidgetAbstraction *abs) {
    mChildren.append(abs);

    if(mContentVisible) {
        mVisiblePartWidget->scheduledUpdateVisibleWidgetsContent();
        mVisiblePartWidget->scheduleUpdateParentHeight();
    }
}

void SingleWidgetAbstraction::addChildAbstractionAt(
        SingleWidgetAbstraction *abs, const int &id) {
    mChildren.insert(id, abs);

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
    removeChildAbstraction(abstraction);
}

void SingleWidgetAbstraction::removeChildAbstraction(
        SingleWidgetAbstraction *abs) {
    mChildren.removeOne(abs);
    if(abs->isDeletable()) {
        delete abs;
    }

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

void SingleWidgetAbstraction::scheduleWidgetContentUpdateIfIsCurrentRule(
        const SWT_Rule &rule) {
    mVisiblePartWidget->scheduleContentUpdateIfIsCurrentRule(rule);
}

void SingleWidgetAbstraction::scheduleWidgetContentUpdateIfIsCurrentTarget(
        const SWT_Target &target) {
    mVisiblePartWidget->scheduleContentUpdateIfIsCurrentTarget(target);
}

void SingleWidgetAbstraction::scheduleWidgetContentUpdateIfSearchNotEmpty() {
    mVisiblePartWidget->scheduleContentUpdateIfSearchNotEmpty();
}

bool SingleWidgetAbstraction::isMainTarget(
        const SWT_RulesCollection &rules) {
    return mTarget->SWT_isMainTarget(rules);
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
