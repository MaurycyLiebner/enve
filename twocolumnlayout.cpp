#include "twocolumnlayout.h"

TwoColumnLayout::TwoColumnLayout() {
    mLayout1 = new QVBoxLayout();
    mLayout2 = new QVBoxLayout();
    addLayout(mLayout1);
    addLayout(mLayout2);
}

void TwoColumnLayout::addWidgetToFirstColumn(QWidget *wid) {
    mLayout1->addWidget(wid);
}

void TwoColumnLayout::addWidgetToSecondColumn(QWidget *wid) {
    mLayout2->addWidget(wid);
}

void TwoColumnLayout::addPair(QWidget *column1, QWidget *column2) {
    addWidgetToFirstColumn(column1);
    addWidgetToSecondColumn(column2);
}
