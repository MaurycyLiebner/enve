#include "minimalscrollwidget.h"
#include "minimalscrollwidgetvisiblepart.h"
#include "singlewidget.h"
#include "singlewidgetabstraction.h"
#include "singlewidgettarget.h"
#include "scrollarea.h"
#include "global.h"

MinimalScrollWidget::MinimalScrollWidget(ScrollArea *parent) :
    QWidget(parent) {
    mParentScrollArea = parent;
    //createVisiblePartWidget();
}

void MinimalScrollWidget::scrollParentAreaBy(const int &by) {
    mParentScrollArea->scrollBy(0, by);
}

void MinimalScrollWidget::setWidth(const int &width) {
    setFixedWidth(width);
    mMinimalVisiblePartWidget->setFixedWidth(width);
    mMinimalVisiblePartWidget->updateWidgetsWidth();
}

void MinimalScrollWidget::changeVisibleTop(const int &top) {
    int newTop = top - top % MIN_WIDGET_HEIGHT;
    mMinimalVisiblePartWidget->move(0, newTop);
    mMinimalVisiblePartWidget->setVisibleTop(newTop);
}

void MinimalScrollWidget::changeVisibleHeight(const int &height) {
    int newHeight = qCeil(height/(qreal)MIN_WIDGET_HEIGHT)*
                    MIN_WIDGET_HEIGHT;
    mMinimalVisiblePartWidget->setVisibleHeight(newHeight);
}

void MinimalScrollWidget::createVisiblePartWidget() {
    mMinimalVisiblePartWidget = new MinimalScrollWidgetVisiblePart(this);
}
