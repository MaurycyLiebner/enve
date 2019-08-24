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

#ifndef MINIMALSCROLLWIDGETVISIBLEPART_H
#define MINIMALSCROLLWIDGETVISIBLEPART_H
class MinimalScrollWidget;
class SingleWidget;
#include <QWidget>

class MinimalScrollWidgetVisiblePart : public QWidget {
public:
    MinimalScrollWidgetVisiblePart(MinimalScrollWidget * const parent);

    void setVisibleTop(const int top);
    void setVisibleHeight(const int height);

    void planScheduleUpdateVisibleWidgetsContent();
    void updateVisibleWidgetsContentIfNeeded();

    void planScheduleUpdateParentHeight();
    void updateParentHeightIfNeeded();

    void updateVisibleWidgets();
    virtual void updateVisibleWidgetsContent();

    void updateParentHeight();

    virtual QWidget *createNewSingleWidget() = 0;
    void updateWidgetsWidth();

    void callUpdaters();

    void scheduleContentUpdate();
protected:
    void postEvent();
    bool event(QEvent* event);

    MinimalScrollWidget *mParentWidget;

    QList<QWidget*> mSingleWidgets;

    int mVisibleTop = 0;
    int mVisibleHeight = 0;
private:
    bool mEventSent = false;
    bool mVisibleWidgetsContentUpdateScheduled = false;
    bool mParentHeightUpdateScheduled = false;
};

#endif // MINIMALSCROLLWIDGETVISIBLEPART_H
