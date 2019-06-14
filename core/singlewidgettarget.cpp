#include "singlewidgettarget.h"
#include "singlewidgetabstraction.h"

//void SingleWidgetTarget::SWT_setupAbstraction(
//        const stdsptr<SingleWidgetAbstraction>&  SWT_Abstraction,
//        const int visiblePartWidgetId) {
//    for(const auto& child : mChildren) {
//        SWT_Abstraction->addChildAbstraction(
//                    child->createAbstraction(visiblePartWidget));
//    }
//}

SingleWidgetAbstraction* SingleWidgetTarget::SWT_createAbstraction(
        const UpdateFuncs& updateFuncs,
        const int visiblePartWidgetId) {
    const auto abs = SPtrCreate(SingleWidgetAbstraction)(
                this, updateFuncs, visiblePartWidgetId);
    SWT_setupAbstraction(abs.get(), updateFuncs, visiblePartWidgetId);
    SWT_mAllAbstractions[visiblePartWidgetId] = abs;
    return abs.get();
}

void SingleWidgetTarget::SWT_addChildAbstractionForTargetToAll(
        SingleWidgetTarget * const target) {
    for(const auto& abs : SWT_mAllAbstractions) {
        abs.second->addChildAbstractionForTarget(target);
    }
}

void SingleWidgetTarget::SWT_addChildAbstractionForTargetToAllAt(
        SingleWidgetTarget * const target, const int id) {
    for(const auto& abs : SWT_mAllAbstractions) {
        abs.second->addChildAbstractionForTargetAt(target, id);
    }
}

void SingleWidgetTarget::SWT_scheduleWidgetsContentUpdateWithRule(
        const SWT_BoxRule &rule) {
    for(const auto& abs : SWT_mAllAbstractions) {
        abs.second->scheduleWidgetContentUpdateIfIsCurrentRule(rule);
    }
}

void SingleWidgetTarget::SWT_scheduleWidgetsContentUpdateWithTarget(
        SingleWidgetTarget *targetP,
        const SWT_Target &target) {
    for(const auto& abs : SWT_mAllAbstractions) {
        abs.second->scheduleWidgetContentUpdateIfIsCurrentTarget(targetP, target);
    }
}

void SingleWidgetTarget::SWT_scheduleWidgetsContentUpdateWithSearchNotEmpty() {
    for(const auto& abs : SWT_mAllAbstractions) {
        abs.second->scheduleWidgetContentUpdateIfSearchNotEmpty();
    }
}

void SingleWidgetTarget::SWT_removeChildAbstractionForTargetFromAll(
        SingleWidgetTarget* const target) {
    for(const auto& abs : SWT_mAllAbstractions) {
        abs.second->removeChildAbstractionForTarget(target);
    }
}

void SingleWidgetTarget::SWT_moveChildAbstractionForTargetToInAll(
        SingleWidgetTarget * const target, const int id) {
    for(const auto& abs : SWT_mAllAbstractions) {
        abs.second->moveChildAbstractionForTargetTo(target, id);
    }
}

void SingleWidgetTarget::SWT_afterContentVisibilityChanged() {
    for(const auto &swa : SWT_mAllAbstractions) {
        swa.second->afterContentVisibilityChanged();
    }
}
