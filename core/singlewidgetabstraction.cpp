#include "singlewidgetabstraction.h"
#include "singlewidgettarget.h"

SingleWidgetAbstraction::SingleWidgetAbstraction(
        const qsptr<SingleWidgetTarget>& target,
        const UpdateFuncs &updateFuncs,
        const int& visiblePartId) :
    mVisiblePartWidgetId(visiblePartId),
    mUpdateFuncs(updateFuncs),
    mTarget(target.data()) {}

SingleWidgetAbstraction::~SingleWidgetAbstraction() {}

bool SingleWidgetAbstraction::getAbstractions(
        const int &minY, const int &maxY,
        int& currY, int currX,
        const int& swtHeight,
        QList<SingleWidgetAbstraction *> &abstractions,
        const SWT_RulesCollection &rules,
        const bool &parentSatisfiesRule,
        const bool &parentMainTarget) { // returns whether should abort
    if(currY > maxY) return true;
    bool satisfiesRule = mTarget->SWT_shouldBeVisible(rules,
                                                      parentSatisfiesRule,
                                                      parentMainTarget);
    if(currY > minY && satisfiesRule && !mIsMainTarget) {
        abstractions.append(this);
    }
    if(satisfiesRule && !mIsMainTarget) {
        currX += swtHeight;
        currY += swtHeight;
    }
    bool childrenVisible = (satisfiesRule && mContentVisible) || mIsMainTarget;
    for(const auto& abs : mChildren) {
        if(abs->getAbstractions(
                    minY, maxY, currY, currX, swtHeight,
                    abstractions, rules,
                    childrenVisible, mIsMainTarget)) {
            return true;
        }
    }

    return false;
}


bool SingleWidgetAbstraction::setSingleWidgetAbstractions(
        const int &minY, const int &maxY,
        int &currY, int currX, const int &swtHeight,
        const SetAbsFunc& setAbsFunc,
        const SWT_RulesCollection &rules,
        const bool &parentSatisfiesRule,
        const bool &parentMainTarget) { // returns whether should abort
    if(!mTarget->SWT_isVisible()) return false;
    if(currY > maxY) return true;
    bool satisfiesRule = mTarget->SWT_shouldBeVisible(rules,
                                                      parentSatisfiesRule,
                                                      parentMainTarget);
    if(currY > minY && satisfiesRule && !mIsMainTarget) {
        setAbsFunc(this, currX);
    }
    if(satisfiesRule && !mIsMainTarget) {
        currX += swtHeight;
        currY += swtHeight;
    }
    bool childrenVisible = (satisfiesRule && mContentVisible) || mIsMainTarget;
    for(const auto& abs : mChildren) {
        if(abs->setSingleWidgetAbstractions(
                    minY, maxY,
                    currY, currX,
                    swtHeight,
                    setAbsFunc,
                    rules,
                    childrenVisible,
                    mIsMainTarget) ) {
            return true;
        }
    }

    return false;
}

int SingleWidgetAbstraction::getHeight(
        const SWT_RulesCollection &rules,
        const bool &parentSatisfiesRule,
        const bool &parentMainTarget,
        const int &swtHeight) {
    int height = 0;
    if(mTarget->SWT_isVisible()) {
        bool satisfiesRule = mTarget->SWT_shouldBeVisible(rules,
                                                          parentSatisfiesRule,
                                                          parentMainTarget);
        if(satisfiesRule && !mIsMainTarget) {
            height += swtHeight;
        }
        bool childrenVisible = (satisfiesRule && mContentVisible) || mIsMainTarget;
        for(const auto& abs : mChildren) {
            height += abs->getHeight(rules, childrenVisible,
                                     mIsMainTarget, swtHeight);
        }
    }

    return height;
}

void SingleWidgetAbstraction::addChildAbstraction(
        SingleWidgetAbstraction* abs) {
    mChildren.append(abs);

    mUpdateFuncs.updateVisibleWidgetsContent();
    mUpdateFuncs.updateParentHeight();
}

void SingleWidgetAbstraction::addChildAbstractionAt(
        SingleWidgetAbstraction* abs, const int &id) {
    mChildren.insert(id, abs);

    mUpdateFuncs.updateVisibleWidgetsContent();
    mUpdateFuncs.updateParentHeight();
}

SingleWidgetAbstraction *SingleWidgetAbstraction::getChildAbstractionForTarget(
        SingleWidgetTarget* target) {
    for(const auto& abs : mChildren) {
        if(abs->getTarget() == target) {
            return abs;
        }
    }
    return nullptr;
}

void SingleWidgetAbstraction::removeChildAbstractionForTarget(
        SingleWidgetTarget* target) {
    auto childAbs = getChildAbstractionForTarget(target);
    if(childAbs) removeChildAbstraction(childAbs);
}

void SingleWidgetAbstraction::removeChildAbstraction(
        SingleWidgetAbstraction* abs) {
    mChildren.removeOne(abs);

    mUpdateFuncs.updateVisibleWidgetsContent();
    mUpdateFuncs.updateParentHeight();
}

void SingleWidgetAbstraction::switchContentVisible() {
    setContentVisible(!mContentVisible);
}

bool SingleWidgetAbstraction::contentVisible() {
    return mContentVisible;
}

void SingleWidgetAbstraction::scheduleWidgetContentUpdateIfIsCurrentRule(
        const SWT_Rule &rule) {
    mUpdateFuncs.contentUpdateIfIsCurrentRule(rule);
}

void SingleWidgetAbstraction::scheduleWidgetContentUpdateIfIsCurrentTarget(
        SingleWidgetTarget* targetP,
        const SWT_Target &target) {
    mUpdateFuncs.contentUpdateIfIsCurrentTarget(targetP, target);
}

void SingleWidgetAbstraction::scheduleWidgetContentUpdateIfSearchNotEmpty() {
    mUpdateFuncs.contentUpdateIfSearchNotEmpty();
}

void SingleWidgetAbstraction::setContentVisible(const bool &bT) {
    //if(bT == mContentVisible) return;
    mContentVisible = bT;
    afterContentVisibilityChanged();
}

SingleWidgetTarget *SingleWidgetAbstraction::getTarget() {
    if(!mTarget) return nullptr;
    return mTarget;
}

void SingleWidgetAbstraction::afterContentVisibilityChanged() {
    mUpdateFuncs.updateParentHeight();
    mUpdateFuncs.updateVisibleWidgetsContent();
}

void SingleWidgetAbstraction::addChildAbstractionForTarget(
        SingleWidgetTarget* target) {
    auto childAbs = target->SWT_getAbstractionForWidget(mUpdateFuncs,
                                                        mVisiblePartWidgetId);
    addChildAbstraction(childAbs);
}

void SingleWidgetAbstraction::addChildAbstractionForTargetAt(
        SingleWidgetTarget *target,
        const int &id) {
    auto childAbs =
            target->SWT_getAbstractionForWidget(mUpdateFuncs,
                                                mVisiblePartWidgetId);
    addChildAbstractionAt(childAbs, id);
}

void SingleWidgetAbstraction::moveChildAbstractionForTargetTo(
        SingleWidgetTarget* target, const int &id) {
    SingleWidgetAbstraction* abs = getChildAbstractionForTarget(target);
    int targetId = id;
//    if(!abs->getTarget()->SWT_visibleOnlyIfParentDescendant()) {
//        for(int i = 0; i < mChildren.count(); i++) {
//            const stdsptr<SingleWidgetAbstraction>& abs1 = mChildren.at(i);
//            if(abs1->getTarget()->SWT_visibleOnlyIfParentDescendant()) {
//                targetId++;
//            } else {
//                break;
//            }
//        }
//    }
    int currId = -1;
    for(int i = 0; i < mChildren.count(); i++) {
        const stdptr<SingleWidgetAbstraction>& absT = mChildren.at(i);
        if(abs == absT) {
            currId = i;
            break;
        }
    }
    if(currId == -1) return;
    mChildren.move(currId, targetId);

    mUpdateFuncs.updateVisibleWidgetsContent();
    mUpdateFuncs.updateParentHeight();
}
