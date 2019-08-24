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

#ifndef SCROLLWIDGET_H
#define SCROLLWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include "minimalscrollwidget.h"
#include "smartPointers/ememory.h"
class SingleWidget;
class ScrollWidgetVisiblePart;
class SWT_Abstraction;
class SingleWidgetTarget;
class ScrollArea;

class ScrollWidget : public MinimalScrollWidget {
    Q_OBJECT
public:
    explicit ScrollWidget(ScrollArea * const parent);

    void updateHeight();
    virtual void updateAbstraction();

    void setMainTarget(SingleWidgetTarget *target);
    int getContentHeight() {
        return mContentHeight;
    }
private:
    void updateHeightAfterScrollAreaResize(const int parentHeight);
protected:
    virtual void createVisiblePartWidget();

    int mContentHeight = 0;
    qptr<SingleWidgetTarget> mMainTarget;
    stdptr<SWT_Abstraction> mMainAbstraction;
    ScrollWidgetVisiblePart *mVisiblePartWidget = nullptr;
};

#endif // SCROLLWIDGET_H
