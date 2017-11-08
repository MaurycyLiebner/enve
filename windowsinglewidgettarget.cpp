#include "windowsinglewidgettarget.h"
#include "canvaswindow.h"
#include "BoxesList/OptimalScrollArea/singlewidgetabstraction.h"

WindowSingleWidgetTarget::WindowSingleWidgetTarget(CanvasWindow *window) {
    mWindow = window;
}

SingleWidgetAbstraction* WindowSingleWidgetTarget::SWT_getAbstractionForWidget(
            ScrollWidgetVisiblePart *visiblePartWidget) {
    Q_FOREACH(const std::shared_ptr<SingleWidgetAbstraction> &abs, mSWT_allAbstractions) {
        if(abs->getParentVisiblePartWidget() == visiblePartWidget) {
            return abs.get();
        }
    }
    SingleWidgetAbstraction *abs = SWT_createAbstraction(visiblePartWidget);
    return abs;
}

void WindowSingleWidgetTarget::SWT_addChildrenAbstractions(
        SingleWidgetAbstraction *abstraction,
        ScrollWidgetVisiblePart *visiblePartWidget) {
    mWindow->SWT_addChildrenAbstractions(abstraction, visiblePartWidget);
}

