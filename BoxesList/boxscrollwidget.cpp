#include "boxscrollwidget.h"

BoxScrollWidget::BoxScrollWidget(QWidget *parent) :
    ScrollWidget(parent) {
    createVisiblePartWidget();
}

BoxScrollWidgetVisiblePart *BoxScrollWidget::getVisiblePartWidget() {
    return (BoxScrollWidgetVisiblePart*)mVisiblePartWidget;
}

void BoxScrollWidget::createVisiblePartWidget() {
    mVisiblePartWidget = new BoxScrollWidgetVisiblePart(this);
}
