#include "boxscrollwidget.h"
#include "GUI/BoxesList/OptimalScrollArea/scrollarea.h"
#include "Boxes/boundingbox.h"
#include "boxscrollwidgetvisiblepart.h"
#include "GUI/canvaswindow.h"
#include "GUI/mainwindow.h"

BoxScrollWidget::BoxScrollWidget(ScrollArea *parent) :
    ScrollWidget(parent) {
    createVisiblePartWidget();
}

//void BoxScrollWidget::updateAbstraction() {
//    if(mMainTarget == nullptr) {
//        mMainAbstraction = nullptr;
//    } else {
//        mMainAbstraction = mMainTarget->
//                SWT_getAbstractionForWidget(mVisiblePartWidget);
//    }
//    mVisiblePartWidget->setMainAbstraction(mMainAbstraction);
//    updateHeight();
//}

BoxScrollWidgetVisiblePart *BoxScrollWidget::getVisiblePartWidget() {
    return (BoxScrollWidgetVisiblePart*)mVisiblePartWidget;
}

void BoxScrollWidget::createVisiblePartWidget() {
    mVisiblePartWidget = new BoxScrollWidgetVisiblePart(this);
    mMinimalVisiblePartWidget = mVisiblePartWidget;
}
