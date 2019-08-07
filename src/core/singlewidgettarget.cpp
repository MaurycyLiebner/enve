#include "singlewidgettarget.h"
#include "singlewidgetabstraction.h"

SWT_Abstraction* SingleWidgetTarget::SWT_createAbstraction(
        const UpdateFuncs& updateFuncs,
        const int visiblePartWidgetId) {
    const auto abs = enve::make_shared<SWT_Abstraction>(
                this, updateFuncs, visiblePartWidgetId);
    SWT_setupAbstraction(abs.get(), updateFuncs, visiblePartWidgetId);
    SWT_mAllAbstractions[visiblePartWidgetId] = abs;
    return abs.get();
}

void SingleWidgetTarget::SWT_addChild(SingleWidgetTarget * const child) {
    for(const auto& abs : SWT_mAllAbstractions) {
        abs.second->addChild(child);
    }
}

void SingleWidgetTarget::SWT_addChildAt(
        SingleWidgetTarget * const child, const int id) {
    for(const auto& abs : SWT_mAllAbstractions) {
        abs.second->addChildAt(child, id);
    }
}

void SingleWidgetTarget::SWT_scheduleContentUpdate(const SWT_BoxRule rule) {
    for(const auto& abs : SWT_mAllAbstractions) {
        abs.second->scheduleContentUpdate(rule);
    }
}

void SingleWidgetTarget::SWT_scheduleSearchContentUpdate() {
    for(const auto& abs : SWT_mAllAbstractions) {
        abs.second->scheduleSearchContentUpdate();
    }
}

void SingleWidgetTarget::SWT_removeChild(
        SingleWidgetTarget * const child) {
    for(const auto& abs : SWT_mAllAbstractions) {
        abs.second->removeChild(child);
    }
}

void SingleWidgetTarget::SWT_moveChildTo(
        SingleWidgetTarget * const child, const int id) {
    for(const auto& abs : SWT_mAllAbstractions) {
        abs.second->moveChildTo(child, id);
    }
}

void SingleWidgetTarget::SWT_afterContentVisibilityChanged() {
    for(const auto &swa : SWT_mAllAbstractions) {
        swa.second->afterContentVisibilityChanged();
    }
}
