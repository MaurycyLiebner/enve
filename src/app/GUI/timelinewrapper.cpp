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

#include "timelinewrapper.h"
#include "timelinewidget.h"
#include "timelinedockwidget.h"

TimelineWrapper::TimelineWrapper(Document * const document,
                                 TWidgetStackLayoutItem* const layItem,
                                 QWidget * const parent) :
    StackWidgetWrapper(
        layItem,
        []() {
            const auto rPtr = new TWidgetStackLayoutItem();
            return std::unique_ptr<WidgetStackLayoutItem>(rPtr);
        },
        [document](WidgetStackLayoutItem* const layItem,
                   QWidget * const parent) {
            const auto tLayItem = static_cast<TWidgetStackLayoutItem*>(layItem);
            const auto derived = new TimelineWrapper(document, tLayItem, parent);
            return static_cast<StackWidgetWrapper*>(derived);
        },
        [document](StackWidgetWrapper * toSetup) {
            const auto newWidget = new TimelineWidget(*document,
                                                      toSetup->getCornerMenu(),
                                                      toSetup);
            toSetup->setCentralWidget(newWidget);
}, parent) {}

void TimelineWrapper::setScene(Canvas * const scene) {
    getTimelineWidget()->setCurrentScene(scene);
}

Canvas* TimelineWrapper::getScene() const {
    return getTimelineWidget()->getCurrrentScene();
}

void TimelineWrapper::saveDataToLayout() const {
    const auto lItem = static_cast<TWidgetStackLayoutItem*>(getLayoutItem());
    const auto sceneWidget = getTimelineWidget();
    lItem->setScene(sceneWidget->getCurrrentScene());
}

TimelineWidget* TimelineWrapper::getTimelineWidget() const {
    return static_cast<TimelineWidget*>(getCentralWidget());
}

void TWidgetStackLayoutItem::clear() {
    SceneWidgetStackLayoutItem::clear();
    mGraph = false;
}

QWidget *TWidgetStackLayoutItem::create(Document &document,
                                        QWidget* const parent,
                                        QLayout * const layout) {
    const auto tWrapper = new TimelineWrapper(&document, this, parent);
    if(layout) {
        tWrapper->disableClose();
        layout->addWidget(tWrapper);
    }
    tWrapper->setScene(mScene);
    const auto tw = tWrapper->getTimelineWidget();
    if(mGraph) tw->setGraphEnabled(mGraph);
    return tWrapper;
}

void TWidgetStackLayoutItem::write(eWriteStream& dst) const {
    SceneWidgetStackLayoutItem::write(dst);
    dst << mGraph;
}

void TWidgetStackLayoutItem::read(eReadStream &src) {
    SceneWidgetStackLayoutItem::read(src);
    src >> mGraph;
}

void TWidgetStackLayoutItem::setGraph(const bool graph) {
    mGraph = graph;
}
