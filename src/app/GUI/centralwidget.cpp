#include "centralwidget.h"
#include "bookmarkedwidget.h"
#include "GUI/global.h"

CentralWidget::CentralWidget(BookmarkedWidget *left,
                             QWidget *central, QWidget *parent) :
    QWidget(parent), mLeft(left), mCentral(central) {
    setContentsMargins(0, 0, 0, 0);
    central->setParent(this);
    left->setParent(this);
    left->move(0, MIN_WIDGET_DIM);
}

void CentralWidget::setSidesVisibilitySetting(const bool vis) {
    mVisibilitySetting = vis;
    updateSideWidgetsVisibility();
}

void CentralWidget::setCanvasMode(const CanvasMode mode) {
    mMode = mode;
    updateSideWidgetsVisibility();
}

void CentralWidget::resizeEvent(QResizeEvent *event) {
    updateSizeWidgetsSize();
    mCentral->resize(event->size());
}

void CentralWidget::updateSideWidgetsVisibility() {
    const bool vis = mMode == CanvasMode::paint && mVisibilitySetting;
    mLeft->setVisible(vis);
    updateSizeWidgetsSize();
}

void CentralWidget::updateSizeWidgetsSize() {
    if(mLeft->isVisible()) {
        mLeft->setMaximumHeight(height() - MIN_WIDGET_DIM);
        mLeft->updateSize();
    }
}
