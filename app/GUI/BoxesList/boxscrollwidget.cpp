#include "boxscrollwidget.h"
#include "GUI/BoxesList/OptimalScrollArea/scrollarea.h"
#include "Boxes/boundingbox.h"
#include "boxscrollwidgetvisiblepart.h"
#include "GUI/canvaswindow.h"
#include "GUI/mainwindow.h"
#include "document.h"

BoxScrollWidget::BoxScrollWidget(Document &document,
                                 ScrollArea * const parent) :
    ScrollWidget(parent) {
    createVisiblePartWidget();
    mCoreAbs = document.SWT_createAbstraction(
                mVisiblePartWidget->getUpdateFuncs(),
                mVisiblePartWidget->getId());
}

BoxScrollWidget::~BoxScrollWidget() {
    if(mCoreAbs) mCoreAbs->removeAlongWithAllChildren_k();
}

//void BoxScrollWidget::updateAbstraction() {
//    if(!mMainTarget) {
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
