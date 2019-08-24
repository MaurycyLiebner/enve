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

#ifndef TIMELINEWRAPPER_H
#define TIMELINEWRAPPER_H
#include "canvaswindowwrapper.h"
class TimelineWidget;
class ChangeWidthWidget;

struct TWidgetStackLayoutItem : public SceneWidgetStackLayoutItem {
    TWidgetStackLayoutItem() {}
    void clear();
    QWidget* create(Document &document,
                    QWidget * const parent,
                    QLayout* const layout = nullptr);
    void write(eWriteStream &dst) const;
    void read(eReadStream& src);
    void setGraph(const bool graph);
private:
    bool mGraph = false;
};

class TimelineWrapper : public StackWidgetWrapper {
public:
    TimelineWrapper(Document * const document,
                    TWidgetStackLayoutItem * const layItem,
                    QWidget * const parent = nullptr);

    void setScene(Canvas* const scene);
    Canvas* getScene() const;
    TimelineWidget *getTimelineWidget() const;

    void saveDataToLayout() const;
private:
    using StackWidgetWrapper::setMenuBar;
    using StackWidgetWrapper::setCentralWidget;
};

#endif // TIMELINEWRAPPER_H
