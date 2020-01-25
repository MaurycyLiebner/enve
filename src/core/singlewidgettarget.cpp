// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "singlewidgettarget.h"
#include "swt_abstraction.h"

SWT_Abstraction* SingleWidgetTarget::SWT_createAbstraction(
        const UpdateFuncs& updateFuncs,
        const int visiblePartWidgetId) {
    const auto curr = SWT_getAbstractionForWidget(visiblePartWidgetId);
    if(curr) return curr;
    const auto abs = enve::make_shared<SWT_Abstraction>(
                this, updateFuncs, visiblePartWidgetId);
    SWT_setupAbstraction(abs.get(), updateFuncs, visiblePartWidgetId);
    SWT_mAllAbstractions[visiblePartWidgetId] = abs;
    return abs.get();
}

void SingleWidgetTarget::SWT_removeAbstractionForWidget(const int visiblePartWidgetId) {
    SWT_mAllAbstractions.erase(visiblePartWidgetId);
}

SWT_Abstraction *SingleWidgetTarget::SWT_getAbstractionForWidget(
        const int visiblePartWidgetId) const {
    const auto it = SWT_mAllAbstractions.find(visiblePartWidgetId);
    if(it == SWT_mAllAbstractions.end()) return nullptr;
    return it->second.get();
}

SWT_Abstraction *SingleWidgetTarget::SWT_abstractionForWidget(
        const UpdateFuncs &updateFuncs, const int visiblePartWidgetId) {
    const auto curr = SWT_getAbstractionForWidget(visiblePartWidgetId);
    if(curr) return curr;
    return SWT_createAbstraction(updateFuncs, visiblePartWidgetId);
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

void SingleWidgetTarget::SWT_setVisible(const bool bT) {
    if(SWT_mVisible == bT) return;
    SWT_mVisible = bT;
    for(const auto &swa : SWT_mAllAbstractions) {
        swa.second->afterContentVisibilityChanged();
    }
}

void SingleWidgetTarget::SWT_setDisabled(const bool disable) {
    if(SWT_mDisabled == disable) return;
    SWT_mDisabled = disable;
    SWT_setChildrenAncestorDisabled(SWT_isDisabled());
    emit SWT_changedDisabled(SWT_isDisabled());
}

void SingleWidgetTarget::SWT_setAncestorDisabled(const bool disabled) {
    if(SWT_mAncestorDisabled == disabled) return;
    SWT_mAncestorDisabled = disabled;
    SWT_setChildrenAncestorDisabled(SWT_isDisabled());
    emit SWT_changedDisabled(SWT_isDisabled());
}
