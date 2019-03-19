#include "boxscrollwidget.h"
#include "GUI/BoxesList/OptimalScrollArea/scrollarea.h"
#include "Boxes/boundingbox.h"
#include "boxscrollwidgetvisiblepart.h"
#include "GUI/canvaswindow.h"
#include "GUI/mainwindow.h"
#include "windowsinglewidgettarget.h"

BoxScrollWidget::BoxScrollWidget(WindowSingleWidgetTarget * const target,
                                 ScrollArea * const parent) :
    ScrollWidget(parent) {
    createVisiblePartWidget();
    mCoreAbs = target->SWT_createAbstraction(
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
