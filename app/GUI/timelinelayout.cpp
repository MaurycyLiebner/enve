#include "timelinelayout.h"
#include "document.h"
#include "boxeslistkeysviewwidget.h"
#include "boxeslistanimationdockwidget.h"
#include "timelinewrapper.h"

TimelineLayout::TimelineLayout(QWidget * const parent) :
    QWidget(parent) {
    setLayout(new QHBoxLayout);
    layout()->setSpacing(0);
    layout()->setMargin(0);

    reset();
}

SceneBaseStackItem* TimelineLayout::extract() {
    const auto tmp = mBaseStack;
    mBaseStack = nullptr;
    return tmp;
}

void TimelineLayout::reset() {
    mBaseStack = nullptr;
    setWidget(nullptr);
}

void TimelineLayout::setCurrent(SceneBaseStackItem* const item) {
    if(!item) return reset();
    setWidget(item->create());
    item->apply();
    mBaseStack = item;
}

void TimelineLayout::saveData() {
    mBaseStack->saveData();
}

void TimelineLayout::setWidget(QWidget * const wid) {
    while(layout()->count() > 0) {
        const auto item = layout()->takeAt(0);
        delete item->widget();
        delete item->layout();
        delete item;
    }
    if(wid) layout()->addWidget(wid);
}
