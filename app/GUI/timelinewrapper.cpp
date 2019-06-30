#include "timelinewrapper.h"
#include "boxeslistkeysviewwidget.h"
#include "boxeslistanimationdockwidget.h"

TimelineWrapper::TimelineWrapper(Document * const document,
                                 TWidgetStackLayoutItem* const layItem,
                                 QWidget * const parent) :
    StackWidgetWrapper(
        layItem,
        [document]() {
            const auto rPtr = new TWidgetStackLayoutItem(*document);
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
    StackWidgetWrapper::saveDataToLayout();
    const auto lItem = static_cast<TWidgetStackLayoutItem*>(getLayoutItem());
    const auto sceneWidget = getTimelineWidget();
    lItem->setScene(sceneWidget->getCurrrentScene());
}

TimelineWidget* TimelineWrapper::getTimelineWidget() const {
    return static_cast<TimelineWidget*>(getCentralWidget());
}

QWidget *TWidgetStackLayoutItem::create() {
    const auto cwWrapper = new TimelineWrapper(&mDocument, this);
    cwWrapper->setScene(mScene);
    return cwWrapper;
}
