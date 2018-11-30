#include "singlewidgettarget.h"
#include "singlewidgetabstraction.h"
#include "GUI/BoxesList/OptimalScrollArea/scrollwidgetvisiblepart.h"

SingleWidgetTarget::SingleWidgetTarget() {
}

SingleWidgetTarget::~SingleWidgetTarget() {
    SWT_clearAll();
}

//void SingleWidgetTarget::SWT_addChildrenAbstractions(
//        const SingleWidgetAbstractionSPtr&  SWT_Abstraction,
//        ScrollWidgetVisiblePart *visiblePartWidget) {
//    Q_FOREACH(const SingleWidgetTargetQSPtr& child, mChildren) {
//        SWT_Abstraction->addChildAbstraction(
//                    child->createAbstraction(visiblePartWidget));
//    }
//}

SingleWidgetAbstraction* SingleWidgetTarget::SWT_createAbstraction(
        ScrollWidgetVisiblePart *visiblePartWidget) {
    SingleWidgetAbstractionSPtr SWT_Abstraction =
            SPtrCreate(SingleWidgetAbstraction)(
                ref<SingleWidgetTarget>(), visiblePartWidget);
    SWT_addChildrenAbstractions(SWT_Abstraction.get(), visiblePartWidget);
    mSWT_allAbstractions << SWT_Abstraction;
    return SWT_Abstraction.get();
}

void SingleWidgetTarget::SWT_removeAbstractionFromList(
        const SingleWidgetAbstractionSPtr &abs) {
    mSWT_allAbstractions.removeOne(abs);
}

void SingleWidgetTarget::SWT_addChildAbstractionForTargetToAll(
        SingleWidgetTarget *target) {
    Q_FOREACH(const SingleWidgetAbstractionSPtr &abs, mSWT_allAbstractions) {
        abs->addChildAbstractionForTarget(target);
    }
}

void SingleWidgetTarget::SWT_addChildAbstractionForTargetToAllAt(
        SingleWidgetTarget* target, const int &id) {
    Q_FOREACH(const SingleWidgetAbstractionSPtr &abs, mSWT_allAbstractions) {
        abs->addChildAbstractionForTargetAt(target, id);
    }
}

void SingleWidgetTarget::SWT_scheduleWidgetsContentUpdateWithRule(
        const SWT_Rule &rule) {
    Q_FOREACH(const SingleWidgetAbstractionSPtr &abs, mSWT_allAbstractions) {
        abs->scheduleWidgetContentUpdateIfIsCurrentRule(rule);
    }
}

void SingleWidgetTarget::SWT_scheduleWidgetsContentUpdateWithTarget(
        SingleWidgetTarget *targetP,
        const SWT_Target &target) {
    Q_FOREACH(const SingleWidgetAbstractionSPtr &abs, mSWT_allAbstractions) {
        abs->scheduleWidgetContentUpdateIfIsCurrentTarget(targetP, target);
    }
}

void SingleWidgetTarget::SWT_scheduleWidgetsContentUpdateWithSearchNotEmpty() {
    Q_FOREACH(const SingleWidgetAbstractionSPtr &abs, mSWT_allAbstractions) {
        abs->scheduleWidgetContentUpdateIfSearchNotEmpty();
    }
}

void SingleWidgetTarget::SWT_removeChildAbstractionForTargetFromAll(
        SingleWidgetTarget* target) {
    Q_FOREACH(const SingleWidgetAbstractionSPtr &abs, mSWT_allAbstractions) {
        abs->removeChildAbstractionForTarget(target);
    }
}

void SingleWidgetTarget::SWT_moveChildAbstractionForTargetToInAll(
        SingleWidgetTarget *target, const int &id) {
    Q_FOREACH(const SingleWidgetAbstractionSPtr &abs, mSWT_allAbstractions) {
        abs->moveChildAbstractionForTargetTo(target, id);
    }
}

void SingleWidgetTarget::SWT_clearAll() {
    mSWT_allAbstractions.clear();
}

void SingleWidgetTarget::SWT_afterContentVisibilityChanged() {
    foreach(const std::shared_ptr<SingleWidgetAbstraction> &swa,
            mSWT_allAbstractions) {
        swa->afterContentVisibilityChanged();
    }
}
