#include "timelinewrapper.h"
#include "boxeslistkeysviewwidget.h"
#include "boxeslistanimationdockwidget.h"

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
