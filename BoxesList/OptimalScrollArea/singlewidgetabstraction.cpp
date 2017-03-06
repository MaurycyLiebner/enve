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

bool SingleWidgetAbstraction::getAbstractions(
        const int &minY, const int &maxY,
        int currY, int currX,
        QList<SingleWidgetAbstraction*> *abstractions,
        const SWT_RulesCollection &rules,
        const bool &parentSatisfiesRule) { // returns whether should abort
    int thisHeight = getHeight(rules,
                               parentSatisfiesRule);
    if(currY + thisHeight < minY) return false;
    if(currY > maxY) return true;
    bool satisfiesRule = mTarget->SWT_satisfiesRule(rules,
                                                    parentSatisfiesRule);
    if(currY > minY && satisfiesRule && !mIsMainTarget) {
        abstractions->append(this);
    }
    if(mContentVisible || mIsMainTarget) {
        if(satisfiesRule && !mIsMainTarget) {
            currX += 20;
            currY += 20;
        }
        foreach(SingleWidgetAbstraction *abs, mChildren) {
            bool childVisible =
                (abs->getTarget()->SWT_visibleOnlyIfParentDescendant()
                 ? !mIsMainTarget
                 : true);
            if(childVisible) {
                if(abs->getAbstractions(minY, maxY,
                                        currY, currX,
                                        abstractions,
                                        rules,
                                        satisfiesRule) ) {
                    return true;
                }
                currY += abs->getHeight(rules,
                                        satisfiesRule);
            }
        }
    }

    return false;
}

bool SingleWidgetAbstraction::setSingleWidgetAbstractions(
        const int &minY, const int &maxY,
        int currY, int currX,
        QList<SingleWidget *> *widgets,
        int *currentWidgetId,
        const SWT_RulesCollection &rules,
        const bool &parentSatisfiesRule) { // returns whether should abort
    int thisHeight = getHeight(rules,
                               parentSatisfiesRule);
    if(currY + thisHeight < minY) return false;
    if(currY > maxY) return true;
    bool satisfiesRule = mTarget->SWT_satisfiesRule(rules,
                                                    parentSatisfiesRule);
    if(currY > minY && satisfiesRule && !mIsMainTarget) {
        if(*currentWidgetId < widgets->count()) {
            SingleWidget *currWidget = widgets->at(*currentWidgetId);
            int currWx = currWidget->x();
            currWidget->move(currX, currWidget->y());
            currWidget->setFixedWidth(currWx - currX + currWidget->width());
            currWidget->setTargetAbstraction(this);
            *currentWidgetId = *currentWidgetId + 1;
        }
    }
    if(mContentVisible || mIsMainTarget) {
        if(satisfiesRule && !mIsMainTarget) {
            currX += 20;
            currY += 20;
        }
        foreach(SingleWidgetAbstraction *abs, mChildren) {
            bool childVisible =
                (abs->getTarget()->SWT_visibleOnlyIfParentDescendant()
                 ? !mIsMainTarget
                 : true);
            if(childVisible) {
                if(abs->setSingleWidgetAbstractions(minY, maxY,
                                                    currY, currX,
                                                    widgets,
                                                    currentWidgetId,
                                                    rules,
                                                    satisfiesRule) ) {
                    return true;
                }
                currY += abs->getHeight(rules,
                                        satisfiesRule);
            }
        }
    }

    return false;
}

int SingleWidgetAbstraction::getHeight(
        const SWT_RulesCollection &rules,
        const bool &parentSatisfiesRule) {
    int height = 0;
    bool satisfiesRule = mTarget->SWT_satisfiesRule(rules,
                                                    parentSatisfiesRule);
    if(satisfiesRule && !mIsMainTarget) {
        height += 20;
    }
    if(mContentVisible || mIsMainTarget) {
        foreach(SingleWidgetAbstraction *abs, mChildren) {
            bool childVisible =
                (abs->getTarget()->SWT_visibleOnlyIfParentDescendant()
                 ? !mIsMainTarget
                 : true);
            if(childVisible) {
                height += abs->getHeight(rules,
                                         satisfiesRule);
            }
        }
    }

    return height;
}

void SingleWidgetAbstraction::addChildAbstraction(
        SingleWidgetAbstraction *abs) {
    mChildren.append(abs);

    if(mContentVisible || mIsMainTarget) {
        mVisiblePartWidget->scheduledUpdateVisibleWidgetsContent();
        mVisiblePartWidget->scheduleUpdateParentHeight();
    }
}

void SingleWidgetAbstraction::addChildAbstractionAt(
        SingleWidgetAbstraction *abs, const int &id) {
    mChildren.insert(id, abs);

    if(mContentVisible || mIsMainTarget) {
        mVisiblePartWidget->scheduledUpdateVisibleWidgetsContent();
        mVisiblePartWidget->scheduleUpdateParentHeight();
    }
}

SingleWidgetAbstraction *SingleWidgetAbstraction::getChildAbstractionForTarget(
        SingleWidgetTarget *target) {
    foreach(SingleWidgetAbstraction *abs, mChildren) {
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
    if(abs->isDeletable()) {
        delete abs;
    }

    if(mContentVisible || mIsMainTarget) {
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

    if(mContentVisible || mIsMainTarget) {
        mVisiblePartWidget->scheduledUpdateVisibleWidgetsContent();
        mVisiblePartWidget->scheduleUpdateParentHeight();
    }
}
