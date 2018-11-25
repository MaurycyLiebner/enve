#include "singlewidgetabstraction.h"
#include "singlewidgettarget.h"
#include "singlewidget.h"
#include "scrollwidgetvisiblepart.h"
#include "global.h"

SingleWidgetAbstraction::SingleWidgetAbstraction(
        const SingleWidgetTargetQSPtr& target,
        ScrollWidgetVisiblePart *visiblePart) {
    mTarget = target.data();
    mVisiblePartWidget = visiblePart;
}

SingleWidgetAbstraction::~SingleWidgetAbstraction() {
//    if(mTarget != nullptr) {
//        mTarget->SWT_removeAbstractionFromList(this);
//    }
//    Q_FOREACH(const SingleWidgetAbstractionSPtr& abs, mChildren) {
//        delete abs;
//    }
}

bool SingleWidgetAbstraction::getAbstractions(
        const int &minY, const int &maxY,
        int& currY, int currX,
        QList<SingleWidgetAbstractionSPtr>& abstractions,
        const SWT_RulesCollection &rules,
        const bool &parentSatisfiesRule,
        const bool &parentMainTarget) { // returns whether should abort
    if(currY > maxY) return true;
    bool satisfiesRule = mTarget->SWT_shouldBeVisible(rules,
                                                      parentSatisfiesRule,
                                                      parentMainTarget);
    if(currY > minY && satisfiesRule && !mIsMainTarget) {
        abstractions.append(ref<SingleWidgetAbstraction>());
    }
    if(satisfiesRule && !mIsMainTarget) {
        currX += MIN_WIDGET_HEIGHT;
        currY += MIN_WIDGET_HEIGHT;
    }
    Q_FOREACH(const std::shared_ptr<SingleWidgetAbstraction> &abs, mChildren) {
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
        QList<QWidget *> *widgets,
        int *currentWidgetId,
        const SWT_RulesCollection &rules,
        const bool &parentSatisfiesRule,
        const bool &parentMainTarget) { // returns whether should abort
    if(!mTarget->SWT_isVisible()) return false;
    if(*currY > maxY) return true;
    bool satisfiesRule = mTarget->SWT_shouldBeVisible(rules,
                                                      parentSatisfiesRule,
                                                      parentMainTarget);
    if(*currY > minY && satisfiesRule && !mIsMainTarget) {
        if(*currentWidgetId < widgets->count()) {
            SingleWidget *currWidget =
                    static_cast<SingleWidget*>(widgets->at(*currentWidgetId));
            currWidget->setTargetAbstraction(this);
            int currWx = currWidget->x();
            currWidget->move(currX, currWidget->y());
            currWidget->setFixedWidth(currWx - currX + currWidget->width());
            *currentWidgetId = *currentWidgetId + 1;
        }
    }
    if(satisfiesRule && !mIsMainTarget) {
        currX += MIN_WIDGET_HEIGHT;
        *currY += MIN_WIDGET_HEIGHT;
    }
    Q_FOREACH(const std::shared_ptr<SingleWidgetAbstraction> &abs, mChildren) {
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
    if(mTarget->SWT_isVisible()) {
        bool satisfiesRule = mTarget->SWT_shouldBeVisible(rules,
                                                          parentSatisfiesRule,
                                                          parentMainTarget);
        if(satisfiesRule && !mIsMainTarget) {
            height += MIN_WIDGET_HEIGHT;
        }
        Q_FOREACH(const std::shared_ptr<SingleWidgetAbstraction> &abs, mChildren) {
            height += abs->getHeight(
                        rules,
                        (satisfiesRule && mContentVisible) || mIsMainTarget,
                        mIsMainTarget);
        }
    }

    return height;
}

void SingleWidgetAbstraction::addChildAbstraction(
        const SingleWidgetAbstractionSPtr& abs) {
    mChildren.append(abs->ref<SingleWidgetAbstraction>());

    mVisiblePartWidget->scheduledUpdateVisibleWidgetsContent();
    mVisiblePartWidget->scheduleUpdateParentHeight();
}

void SingleWidgetAbstraction::addChildAbstractionAt(
        const SingleWidgetAbstractionSPtr& abs, const int &id) {
    mChildren.insert(id, abs->ref<SingleWidgetAbstraction>());

    mVisiblePartWidget->scheduledUpdateVisibleWidgetsContent();
    mVisiblePartWidget->scheduleUpdateParentHeight();
}

SingleWidgetAbstraction *SingleWidgetAbstraction::getChildAbstractionForTarget(
        SingleWidgetTarget* target) {
    Q_FOREACH(const SingleWidgetAbstractionSPtr &abs, mChildren) {
        if(abs->getTarget() == target) {
            return abs.get();
        }
    }
    return nullptr;
}

void SingleWidgetAbstraction::removeChildAbstractionForTarget(
        SingleWidgetTarget* target) {
    auto chidAbs = getChildAbstractionForTarget(target);
    removeChildAbstraction(chidAbs->ref<SingleWidgetAbstraction>());
}

void SingleWidgetAbstraction::removeChildAbstraction(
        const SingleWidgetAbstractionSPtr& abs) {
    for(int i = 0; i < mChildren.count(); i++) {
        const std::shared_ptr<SingleWidgetAbstraction> &absT = mChildren.at(i);
        if(abs == absT) {
            mChildren.removeAt(i);
            break;
        }
    }

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
        SingleWidgetTarget* targetP,
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
    afterContentVisibilityChanged();
}

SingleWidgetTarget *SingleWidgetAbstraction::getTarget() {
    if(mTarget == nullptr) return nullptr;
    return mTarget;
}

void SingleWidgetAbstraction::afterContentVisibilityChanged() {
    mVisiblePartWidget->scheduleUpdateParentHeight();
    mVisiblePartWidget->scheduledUpdateVisibleWidgetsContent();
}

void SingleWidgetAbstraction::addChildAbstractionForTarget(
        SingleWidgetTarget* target) {
    addChildAbstraction(
                target->SWT_getAbstractionForWidget(mVisiblePartWidget));
}

void SingleWidgetAbstraction::addChildAbstractionForTargetAt(SingleWidgetTarget *target, const int &id) {
    addChildAbstractionAt(target->SWT_getAbstractionForWidget(mVisiblePartWidget), id);
}

void SingleWidgetAbstraction::moveChildAbstractionForTargetTo(
        SingleWidgetTarget* target, const int &id) {
    SingleWidgetAbstraction* abs = getChildAbstractionForTarget(target);
    int targetId = id;
//    if(!abs->getTarget()->SWT_visibleOnlyIfParentDescendant()) {
//        for(int i = 0; i < mChildren.count(); i++) {
//            const SingleWidgetAbstractionSPtr& abs1 = mChildren.at(i);
//            if(abs1->getTarget()->SWT_visibleOnlyIfParentDescendant()) {
//                targetId++;
//            } else {
//                break;
//            }
//        }
//    }
    int currId = -1;
    for(int i = 0; i < mChildren.count(); i++) {
        const SingleWidgetAbstractionSPtr& absT = mChildren.at(i);
        if(abs == absT.get()) {
            currId = i;
            break;
        }
    }
    if(currId == -1) return;
    mChildren.move(currId, targetId);

    mVisiblePartWidget->scheduledUpdateVisibleWidgetsContent();
    mVisiblePartWidget->scheduleUpdateParentHeight();
}
