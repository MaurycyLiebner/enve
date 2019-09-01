// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

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
#include "singlewidgetabstraction.h"

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
