#include "singlewidgetabstraction.h"
#include "singlewidgettarget.h"

SingleWidgetAbstraction::SingleWidgetAbstraction(
        const qsptr<SingleWidgetTarget>& target,
        const UpdateFuncs &updateFuncs,
        const int visiblePartId) :
    mVisiblePartWidgetId(visiblePartId),
    mUpdateFuncs(updateFuncs),
    mTarget(target.data()) {}

SingleWidgetAbstraction::~SingleWidgetAbstraction() {}

bool SingleWidgetAbstraction::getAbstractions(
        const int minY, const int maxY,
        int& currY, int currX,
        const int swtHeight,
        QList<SingleWidgetAbstraction *> &abstractions,
        const SWT_RulesCollection &rules,
        const bool parentSatisfiesRule,
        const bool parentMainTarget) { // returns whether should abort
    if(currY > maxY) return true;
    const bool satisfiesRule = mTarget->SWT_isVisible() &&
            mTarget->SWT_shouldBeVisible(rules, parentSatisfiesRule,
                                         parentMainTarget);
    if(currY > minY && satisfiesRule && !mIsMainTarget) {
        abstractions.append(this);
    }
    if(satisfiesRule && !mIsMainTarget) {
        currX += swtHeight;
        currY += swtHeight;
    }
    const bool childrenVisible = (satisfiesRule && mContentVisible) ||
                                 mIsMainTarget;
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
        const int minY, const int maxY,
        int &currY, int currX, const int swtHeight,
        const SetAbsFunc& setAbsFunc,
        const SWT_RulesCollection &rules,
        const bool parentSatisfiesRule,
        const bool parentMainTarget) { // returns whether should abort
    if(!mTarget->SWT_isVisible()) return false;
    if(currY > maxY) return true;
    const bool satisfiesRule = mTarget->SWT_isVisible() &&
            mTarget->SWT_shouldBeVisible(
                rules, parentSatisfiesRule, parentMainTarget);
    if(currY > minY && satisfiesRule && !mIsMainTarget) {
        setAbsFunc(this, currX);
    }
    if(satisfiesRule && !mIsMainTarget) {
        currX += swtHeight;
        currY += swtHeight;
    }
    const bool childrenVisible = (satisfiesRule && mContentVisible) ||
                                 mIsMainTarget;
    for(const auto& abs : mChildren) {
        if(abs->setSingleWidgetAbstractions(minY, maxY, currY, currX,
                                            swtHeight, setAbsFunc, rules,
                                            childrenVisible, mIsMainTarget)) {
            return true;
        }
    }

    return false;
}

int SingleWidgetAbstraction::getHeight(
        const SWT_RulesCollection &rules,
        const bool parentSatisfiesRule,
        const bool parentMainTarget,
        const int swtHeight) {
    int height = 0;
    if(mTarget->SWT_isVisible()) {
        const bool satisfiesRule = mTarget->SWT_isVisible() &&
                mTarget->SWT_shouldBeVisible(rules, parentSatisfiesRule,
                                             parentMainTarget);
        if(satisfiesRule && !mIsMainTarget) {
            height += swtHeight;
        }
        const bool childrenVisible = (satisfiesRule && mContentVisible) ||
                                     mIsMainTarget;
        for(const auto& abs : mChildren) {
            height += abs->getHeight(rules, childrenVisible,
                                     mIsMainTarget, swtHeight);
        }
    }

    return height;
}

void SingleWidgetAbstraction::addChildAbstraction(
        SingleWidgetAbstraction * const abs) {
    addChildAbstractionAt(abs, mChildren.count());
}

void SingleWidgetAbstraction::addChildAbstractionAt(
        SingleWidgetAbstraction* const abs, const int id) {
    mChildren.insert(id, abs);
    abs->setParent(this);
    updateChildrenIds(id, mChildren.count() - 1);

    mUpdateFuncs.fUpdateVisibleWidgetsContent();
    mUpdateFuncs.fUpdateParentHeight();
}

SingleWidgetAbstraction *SingleWidgetAbstraction::getChildAbstractionForTarget(
        const SingleWidgetTarget* const target) {
    for(const auto& abs : mChildren) {
        if(abs->getTarget() == target) return abs;
    }
    return nullptr;
}

void SingleWidgetAbstraction::removeChildAbstractionForTarget(
        const SingleWidgetTarget* const target) {
    auto childAbs = getChildAbstractionForTarget(target);
    if(childAbs) removeChildAbstraction(childAbs);
}

void SingleWidgetAbstraction::removeChildAbstraction(
        SingleWidgetAbstraction * const abs) {
    const int currId = abs->getIdInParent();
    mChildren.removeOne(abs);
    if(abs->getParent() == this) abs->setParent(nullptr);
    updateChildrenIds(currId, mChildren.count() - 1);

    mUpdateFuncs.fUpdateVisibleWidgetsContent();
    mUpdateFuncs.fUpdateParentHeight();
}

void SingleWidgetAbstraction::switchContentVisible() {
    setContentVisible(!mContentVisible);
}

bool SingleWidgetAbstraction::contentVisible() {
    return mContentVisible;
}

void SingleWidgetAbstraction::scheduleWidgetContentUpdateIfIsCurrentRule(
        const SWT_BoxRule &rule) {
    mUpdateFuncs.fContentUpdateIfIsCurrentRule(rule);
}

void SingleWidgetAbstraction::scheduleWidgetContentUpdateIfIsCurrentTarget(
        SingleWidgetTarget * const targetP,
        const SWT_Target &target) {
    mUpdateFuncs.fContentUpdateIfIsCurrentTarget(targetP, target);
}

void SingleWidgetAbstraction::scheduleWidgetContentUpdateIfSearchNotEmpty() {
    mUpdateFuncs.fContentUpdateIfSearchNotEmpty();
}

void SingleWidgetAbstraction::setContentVisible(const bool bT) {
    //if(bT == mContentVisible) return;
    mContentVisible = bT;
    afterContentVisibilityChanged();
}

SingleWidgetTarget *SingleWidgetAbstraction::getTarget() const {
    return mTarget;
}

void SingleWidgetAbstraction::afterContentVisibilityChanged() {
    mUpdateFuncs.fUpdateParentHeight();
    mUpdateFuncs.fUpdateVisibleWidgetsContent();
}

void SingleWidgetAbstraction::removeAlongWithAllChildren_k() {
    for(const auto& child : mChildren)
        child->removeAlongWithAllChildren_k();
    if(mTarget) mTarget->SWT_removeAbstractionFromList(
                ref<SingleWidgetAbstraction>());
}

void SingleWidgetAbstraction::addChildAbstractionForTarget(
        SingleWidgetTarget * const target) {
    auto childAbs = target->SWT_getOrCreateAbstractionForWidget(
                mUpdateFuncs, mVisiblePartWidgetId);
    addChildAbstraction(childAbs);
}

void SingleWidgetAbstraction::addChildAbstractionForTargetAt(
        SingleWidgetTarget * const target,
        const int id) {
    auto childAbs = target->SWT_getOrCreateAbstractionForWidget(mUpdateFuncs,
                                                        mVisiblePartWidgetId);
    addChildAbstractionAt(childAbs, id);
}

void SingleWidgetAbstraction::moveChildAbstractionForTargetTo(
        SingleWidgetTarget * const target, const int id) {
    const auto abs = getChildAbstractionForTarget(target);
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
        const auto& absT = mChildren.at(i);
        if(abs == absT) {
            currId = i;
            break;
        }
    }
    if(currId == -1) return;
    mChildren.move(currId, targetId);
    updateChildrenIds(qMin(currId, targetId), qMax(currId, targetId));

    mUpdateFuncs.fUpdateVisibleWidgetsContent();
    mUpdateFuncs.fUpdateParentHeight();
}
