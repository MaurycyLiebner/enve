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

#include "scrollwidget.h"
#include "scrollwidgetvisiblepart.h"
#include "singlewidget.h"
#include "singlewidgetabstraction.h"
#include "singlewidgettarget.h"
#include "scrollarea.h"
#include "GUI/global.h"

ScrollWidget::ScrollWidget(ScrollArea * const parent) :
    MinimalScrollWidget(parent) {
    //createVisiblePartWidget();
    connect(parent, &ScrollArea::heightChanged,
            this, &ScrollWidget::updateHeightAfterScrollAreaResize);
}

void ScrollWidget::updateAbstraction() {
    if(mMainAbstraction) {
        mMainAbstraction->setIsMainTarget(false);
//        if(mMainAbstraction->isDeletable()) {
//            delete mMainAbstraction;
//        }
    }
    if(!mMainTarget) {
        mMainAbstraction = nullptr;
    } else {
        int widId = mVisiblePartWidget->getId();
        mMainAbstraction = mMainTarget->SWT_getAbstractionForWidget(widId);
        mMainAbstraction->setIsMainTarget(true);
    }
    mVisiblePartWidget->setMainAbstraction(mMainAbstraction);
    updateHeight();
}

void ScrollWidget::createVisiblePartWidget() {
    mVisiblePartWidget = new ScrollWidgetVisiblePart(this);
    mMinimalVisiblePartWidget = mVisiblePartWidget;
}

void ScrollWidget::setMainTarget(SingleWidgetTarget *target) {
    mMainTarget = target;
    updateAbstraction();
}

void ScrollWidget::updateHeight() {
    if(!mMainAbstraction) return hide();
    mContentHeight = mMainAbstraction->updateHeight(
                mVisiblePartWidget->getCurrentRulesCollection(),
                false, false, MIN_WIDGET_DIM) + MIN_WIDGET_DIM;
    const int parentHeight = mParentScrollArea->height();
    setFixedHeight(qMax(mContentHeight, parentHeight));
    if(isHidden()) show();
}

void ScrollWidget::updateHeightAfterScrollAreaResize(const int parentHeight) {
    setFixedHeight(qMax(mContentHeight, parentHeight));
}
