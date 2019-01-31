#include "singlewidgettarget.h"
#include "singlewidgetabstraction.h"

SingleWidgetTarget::SingleWidgetTarget() {
}

SingleWidgetTarget::~SingleWidgetTarget() {
}

//void SingleWidgetTarget::SWT_addChildrenAbstractions(
//        const stdsptr<SingleWidgetAbstraction>&  SWT_Abstraction,
//        const int& visiblePartWidgetId) {
//    for(const auto& child : mChildren) {
//        SWT_Abstraction->addChildAbstraction(
//                    child->createAbstraction(visiblePartWidget));
//    }
//}

SingleWidgetAbstraction* SingleWidgetTarget::SWT_createAbstraction(
        const UpdateFuncs& updateFuncs,
        const int& visiblePartWidgetId) {
    stdsptr<SingleWidgetAbstraction> SWT_Abstraction =
            SPtrCreate(SingleWidgetAbstraction)(
                ref<SingleWidgetTarget>(), updateFuncs, visiblePartWidgetId);
    SWT_addChildrenAbstractions(SWT_Abstraction.get(),
                                updateFuncs,
                                visiblePartWidgetId);
    mSWT_allAbstractions << SWT_Abstraction;
    return SWT_Abstraction.get();
}

void SingleWidgetTarget::SWT_removeAbstractionFromList(
        const stdsptr<SingleWidgetAbstraction> &abs) {
    mSWT_allAbstractions.removeOne(abs);
}

void SingleWidgetTarget::SWT_addChildAbstractionForTargetToAll(
        SingleWidgetTarget *target) {
    for(const auto& abs : mSWT_allAbstractions) {
        abs->addChildAbstractionForTarget(target);
    }
}

void SingleWidgetTarget::SWT_addChildAbstractionForTargetToAllAt(
        SingleWidgetTarget* target, const int &id) {
    for(const auto& abs : mSWT_allAbstractions) {
        abs->addChildAbstractionForTargetAt(target, id);
    }
}

void SingleWidgetTarget::SWT_scheduleWidgetsContentUpdateWithRule(
        const SWT_Rule &rule) {
    for(const auto& abs : mSWT_allAbstractions) {
        abs->scheduleWidgetContentUpdateIfIsCurrentRule(rule);
    }
}

void SingleWidgetTarget::SWT_scheduleWidgetsContentUpdateWithTarget(
        SingleWidgetTarget *targetP,
        const SWT_Target &target) {
    for(const auto& abs : mSWT_allAbstractions) {
        abs->scheduleWidgetContentUpdateIfIsCurrentTarget(targetP, target);
    }
}

void SingleWidgetTarget::SWT_scheduleWidgetsContentUpdateWithSearchNotEmpty() {
    for(const auto& abs : mSWT_allAbstractions) {
        abs->scheduleWidgetContentUpdateIfSearchNotEmpty();
    }
}

void SingleWidgetTarget::SWT_removeChildAbstractionForTargetFromAll(
        SingleWidgetTarget* target) {
    for(const auto& abs : mSWT_allAbstractions) {
        abs->removeChildAbstractionForTarget(target);
    }
}

void SingleWidgetTarget::SWT_moveChildAbstractionForTargetToInAll(
        SingleWidgetTarget *target, const int &id) {
    for(const auto& abs : mSWT_allAbstractions) {
        abs->moveChildAbstractionForTargetTo(target, id);
    }
}

void SingleWidgetTarget::SWT_afterContentVisibilityChanged() {
    foreach(const stdsptr<SingleWidgetAbstraction> &swa,
            mSWT_allAbstractions) {
        swa->afterContentVisibilityChanged();
    }
}
