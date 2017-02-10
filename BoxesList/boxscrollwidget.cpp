#include "boxscrollwidget.h"
#include "Boxes/boundingbox.h"

BoxScrollWidget::BoxScrollWidget(QWidget *parent) :
    ScrollWidget(parent) {
    createVisiblePartWidget();
}

void BoxScrollWidget::updateAbstraction() {
    if(mMainTarget == NULL) {
        mMainAbstraction = NULL;
    } else {
        mMainAbstraction = ((BoundingBox*)mMainTarget)->getTargetAbstraction();
    }
    mVisiblePartWidget->setMainAbstraction(mMainAbstraction);
    updateHeight();
}

BoxScrollWidgetVisiblePart *BoxScrollWidget::getVisiblePartWidget() {
    return (BoxScrollWidgetVisiblePart*)mVisiblePartWidget;
}

void BoxScrollWidget::createVisiblePartWidget() {
    mVisiblePartWidget = new BoxScrollWidgetVisiblePart(this);
}
