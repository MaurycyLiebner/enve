#include "timelinelayout.h"
#include "document.h"
#include "boxeslistkeysviewwidget.h"
#include "boxeslistanimationdockwidget.h"
#include "timelinewrapper.h"

TimelineLayout::TimelineLayout(Document& document,
                               QWidget * const parent) :
    QWidget(parent),
    mDocument(document) {
    setLayout(new QHBoxLayout);
    layout()->setSpacing(0);
    layout()->setMargin(0);

    reset();
}

SceneBaseStackItem::cUPtr TimelineLayout::extract() {
    return std::move(mBaseStack);
}

void TimelineLayout::reset(TimelineWrapper** const cwwP) {
    mBaseStack = std::make_unique<TSceneBaseStackItem>();
    const auto cwwItem = static_cast<TWidgetStackLayoutItem*>(
                mBaseStack->getChild());
    const auto cww = new TimelineWrapper(&mDocument, cwwItem);
    cww->disableClose();
    if(cwwP) *cwwP = cww;
    else setWidget(cww);
}

void TimelineLayout::setCurrent(const SceneBaseStackItem* const item) {
    TimelineWrapper* cwwP = nullptr;
    if(!item) return reset();
    reset(&cwwP);
    item->apply(cwwP);
    QWidget* mainW = cwwP;
    while(mainW->parentWidget())
        mainW = mainW->parentWidget();
    setWidget(mainW);
    mBaseStack->setScene(item->getScene());
}


void TimelineLayout::setWidget(QWidget * const wid) {
    while(layout()->count() > 0) {
        const auto item = layout()->takeAt(0);
        delete item->widget();
        delete item->layout();
        delete item;
    }
    layout()->addWidget(wid);
}
