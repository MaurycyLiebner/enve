#include "boxscrollwidget.h"

BoxScrollWidget::BoxScrollWidget(QWidget *parent) :
    ScrollWidget(parent) {
    createVisiblePartWidget();
}

void BoxScrollWidget::createVisiblePartWidget() {
    mVisiblePartWidget = new BoxScrollWidgetVisiblePart(this);
}
