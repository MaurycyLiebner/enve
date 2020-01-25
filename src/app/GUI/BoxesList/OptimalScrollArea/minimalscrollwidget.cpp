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

#include "minimalscrollwidget.h"
#include "minimalscrollwidgetvisiblepart.h"
#include "singlewidget.h"
#include "swt_abstraction.h"
#include "singlewidgettarget.h"
#include "scrollarea.h"
#include "GUI/global.h"

MinimalScrollWidget::MinimalScrollWidget(ScrollArea * const parent) :
    QWidget(parent) {
    mParentScrollArea = parent;
    //createVisiblePartWidget();
}

void MinimalScrollWidget::scrollParentAreaBy(const int by) {
    mParentScrollArea->scrollBy(0, by);
}

void MinimalScrollWidget::setWidth(const int width) {
    setFixedWidth(width);
    mMinimalVisiblePartWidget->setFixedWidth(width);
    mMinimalVisiblePartWidget->updateWidgetsWidth();
}

void MinimalScrollWidget::changeVisibleTop(const int top) {
    int newTop = top - top % MIN_WIDGET_DIM;
    mMinimalVisiblePartWidget->move(0, newTop);
    mMinimalVisiblePartWidget->setVisibleTop(newTop);
}

void MinimalScrollWidget::changeVisibleHeight(const int height) {
    int newHeight = qCeil(height/(qreal)MIN_WIDGET_DIM)*
                    MIN_WIDGET_DIM;
    mMinimalVisiblePartWidget->setVisibleHeight(newHeight);
}
