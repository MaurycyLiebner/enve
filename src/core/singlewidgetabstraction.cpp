#include "singlewidgetabstraction.h"
#include "singlewidgettarget.h"

SWT_Abstraction::SWT_Abstraction(
        SingleWidgetTarget * const target,
        const UpdateFuncs &updateFuncs,
        const int visiblePartId) :
    mVisiblePartWidgetId(visiblePartId),
    mUpdateFuncs(updateFuncs),
    mTarget_k(target) {}

bool SWT_Abstraction::setAbstractions(
        const int minY, const int maxY,
        int &currY, int currX, const int swtHeight,
        const SetAbsFunc& setAbsFunc,
        const SWT_RulesCollection &rules,
        const bool parentSatisfiesRule,
        const bool parentMainTarget) { // returns whether should abort
    if(!mTarget_k->SWT_isVisible()) return false;
    if(currY > maxY) return true;
    const bool satisfiesRule = mTarget_k->SWT_shouldBeVisible(
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
        if(abs->setAbstractions(minY, maxY, currY, currX,
                                swtHeight, setAbsFunc, rules,
                                childrenVisible, mIsMainTarget)) {
            return true;
        }
    }

    return false;
}

int SWT_Abstraction::updateHeight(const SWT_RulesCollection &rules,
                                  const bool parentSatisfiesRule,
                                  const bool parentMainTarget,
                                  const int swtHeight) {
    mHeight = 0;
    if(mTarget_k->SWT_isVisible()) {
        const bool satisfiesRule = mTarget_k->SWT_shouldBeVisible(
                    rules, parentSatisfiesRule, parentMainTarget);
        if(satisfiesRule && !mIsMainTarget)
            mHeight += swtHeight;
        const bool childrenVisible = (satisfiesRule && mContentVisible) ||
                                     mIsMainTarget;
        for(const auto& abs : mChildren) {
            mHeight += abs->updateHeight(rules, childrenVisible,
                                         mIsMainTarget, swtHeight);
        }
    }

    return mHeight;
}

void SWT_Abstraction::addChildAbstraction(
        SWT_Abstraction * const abs) {
    addChildAbstractionAt(abs, mChildren.count());
}

void SWT_Abstraction::addChildAbstractionAt(
        SWT_Abstraction* const abs, const int id) {
    mChildren.insert(id, abs);
    abs->setParent(this);
    updateChildrenIds(id, mChildren.count() - 1);

    mUpdateFuncs.fUpdateVisibleWidgetsContent();
    mUpdateFuncs.fUpdateParentHeight();
}

SWT_Abstraction *SWT_Abstraction::getChildAbsFor(
        const SingleWidgetTarget* const target) {
    for(const auto& abs : mChildren) {
        if(abs->getTarget() == target) return abs;
    }
    return nullptr;
}

void SWT_Abstraction::removeChild(
        const SingleWidgetTarget* const target) {
    auto childAbs = getChildAbsFor(target);
    if(childAbs) removeChild(childAbs);
}

void SWT_Abstraction::removeChild(
        SWT_Abstraction * const abs) {
    const int currId = abs->getIdInParent();
    mChildren.removeOne(abs);
    if(abs->getParent() == this) abs->setParent(nullptr);
    updateChildrenIds(currId, mChildren.count() - 1);

    mUpdateFuncs.fUpdateVisibleWidgetsContent();
    mUpdateFuncs.fUpdateParentHeight();
}

void SWT_Abstraction::switchContentVisible() {
    setContentVisible(!mContentVisible);
}

bool SWT_Abstraction::contentVisible() {
    return mContentVisible;
}

void SWT_Abstraction::scheduleContentUpdate(const SWT_BoxRule rule) {
    mUpdateFuncs.fContentUpdateIfIsCurrentRule(rule);
}

void SWT_Abstraction::scheduleSearchContentUpdate() {
    mUpdateFuncs.fContentUpdateIfSearchNotEmpty();
}

void SWT_Abstraction::setContentVisible(const bool bT) {
    //if(bT == mContentVisible) return;
    mContentVisible = bT;
    afterContentVisibilityChanged();
}

SingleWidgetTarget *SWT_Abstraction::getTarget() const {
    return mTarget_k;
}

void SWT_Abstraction::afterContentVisibilityChanged() {
    mUpdateFuncs.fUpdateParentHeight();
    mUpdateFuncs.fUpdateVisibleWidgetsContent();
}

void SWT_Abstraction::removeAlongWithAllChildren_k() {
    for(const auto& child : mChildren)
        child->removeAlongWithAllChildren_k();
    if(mTarget_k)
        mTarget_k->SWT_removeAbstractionForWidget(mVisiblePartWidgetId);
}

void SWT_Abstraction::addChild(
        SingleWidgetTarget * const target) {
    auto childAbs = target->SWT_abstractionForWidget(
                mUpdateFuncs, mVisiblePartWidgetId);
    addChildAbstraction(childAbs);
}

void SWT_Abstraction::addChildAt(
        SingleWidgetTarget * const target,
        const int id) {
    auto childAbs = target->SWT_abstractionForWidget(mUpdateFuncs,
                                                        mVisiblePartWidgetId);
    addChildAbstractionAt(childAbs, id);
}

void SWT_Abstraction::moveChildTo(
        SingleWidgetTarget * const target, const int id) {
    const auto abs = getChildAbsFor(target);
    int targetId = id;
//    if(!abs->getTarget()->SWT_visibleOnlyIfParentDescendant()) {
//        for(int i = 0; i < mChildren.count(); i++) {
//            const stdsptr<SWT_Abstraction>& abs1 = mChildren.at(i);
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
