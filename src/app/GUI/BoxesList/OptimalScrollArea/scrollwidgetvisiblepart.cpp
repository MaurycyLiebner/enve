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

#include "scrollwidgetvisiblepart.h"
#include <QPainter>
#include "singlewidgetabstraction.h"
#include "singlewidget.h"
#include "scrollwidget.h"
#include "singlewidgettarget.h"
#include "GUI/mainwindow.h"
#include "GUI/global.h"

int ScrollWidgetVisiblePart::sNextId = 0;

ScrollWidgetVisiblePart::ScrollWidgetVisiblePart(ScrollWidget * const parent) :
    MinimalScrollWidgetVisiblePart(parent), mId(sNextId++) {
    mCurrentRulesCollection.fRule = SWT_BR_ALL;
    setupUpdateFuncs();
}

void ScrollWidgetVisiblePart::setCurrentRule(const SWT_BoxRule rule) {
    mCurrentRulesCollection.fRule = rule;
    updateParentHeight();
    updateVisibleWidgetsContent();
}

void ScrollWidgetVisiblePart::setCurrentTarget(SingleWidgetTarget* targetP,
                                               const SWT_Target target) {
    mCurrentRulesCollection.fTarget = target;
    static_cast<ScrollWidget*>(mParentWidget)->setMainTarget(targetP);
    updateParentHeight();
    updateVisibleWidgetsContent();
}

void ScrollWidgetVisiblePart::setCurrentType(const SWT_Type type) {
    mCurrentRulesCollection.fType = type;
    updateParentHeight();
    updateVisibleWidgetsContent();
}

void ScrollWidgetVisiblePart::setAlwaysShowChildren(const bool alwaysShowChildren) {
    mCurrentRulesCollection.fAlwaysShowChildren = alwaysShowChildren;
    updateParentHeight();
    updateVisibleWidgetsContent();
}

void ScrollWidgetVisiblePart::setCurrentSearchText(const QString &text) {
    mCurrentRulesCollection.fSearchString = text;
    updateParentHeight();
    updateVisibleWidgetsContent();
}

void ScrollWidgetVisiblePart::scheduleContentUpdateIfIsCurrentRule(const SWT_BoxRule rule) {
    if(isCurrentRule(rule)) {
        planScheduleUpdateParentHeight();
        planScheduleUpdateVisibleWidgetsContent();
    }
}

void ScrollWidgetVisiblePart::scheduleContentUpdateIfIsCurrentTarget(
        SingleWidgetTarget* targetP, const SWT_Target target) {
    if(mCurrentRulesCollection.fTarget == target) {
        static_cast<ScrollWidget*>(mParentWidget)->setMainTarget(targetP);
        updateParentHeight();
        updateVisibleWidgetsContent();
        //planScheduleUpdateParentHeight();
        //planScheduleUpdateVisibleWidgetsContent();
    }
}

void ScrollWidgetVisiblePart::scheduleContentUpdateIfSearchNotEmpty() {
    if(mCurrentRulesCollection.fSearchString.isEmpty()) return;
    planScheduleUpdateParentHeight();
    planScheduleUpdateVisibleWidgetsContent();
}

bool ScrollWidgetVisiblePart::isCurrentRule(const SWT_BoxRule rule) {
    return rule == mCurrentRulesCollection.fRule;
}

void ScrollWidgetVisiblePart::updateVisibleWidgetsContent() {
    if(!mMainAbstraction) {
        for(const auto& wid : mSingleWidgets) wid->hide();
        return;
    }
    //updateParentHeight();

    int currentWidgetId = 0;
    SetAbsFunc setAbsFunc = [this, &currentWidgetId](
            SWT_Abstraction* newAbs, const int currX) {
        if(currentWidgetId < mSingleWidgets.count()) {
            const auto currWidget = static_cast<SingleWidget*>(
                        mSingleWidgets.at(currentWidgetId));
            currWidget->setTargetAbstraction(newAbs);
            const int currWx = currWidget->x();
            currWidget->move(currX, currWidget->y());
            currWidget->setFixedWidth(currWx - currX + currWidget->width());
            currentWidgetId = currentWidgetId + 1;
        }
    };
    int currY = MIN_WIDGET_DIM/2;
    mMainAbstraction->setAbstractions(
                mVisibleTop,
                mVisibleTop + mVisibleHeight + currY,
                currY, 0, MIN_WIDGET_DIM,
                setAbsFunc,
                mCurrentRulesCollection,
                true, false);

    mNVisible = currentWidgetId;
    for(int i = currentWidgetId; i < mSingleWidgets.count(); i++) {
        mSingleWidgets.at(i)->hide();
    }
}

void ScrollWidgetVisiblePart::setMainAbstraction(SWT_Abstraction* abs) {
    mMainAbstraction = abs;
    planScheduleUpdateVisibleWidgetsContent();
//    if(!abs) return;
//    abs->setContentVisible(true);
//    updateVisibleWidgetsContent();
//    updateParentHeight();
}

QWidget *ScrollWidgetVisiblePart::createNewSingleWidget() {
    return new SingleWidget(this);
}

SWT_RulesCollection::SWT_RulesCollection() {
    fType = SWT_TYPE_ALL;
    fRule = SWT_BR_ALL;
    fAlwaysShowChildren = false;
    fTarget = SWT_TARGET_CURRENT_CANVAS;
    fSearchString = "";
}
