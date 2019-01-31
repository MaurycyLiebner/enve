#include "windowsinglewidgettarget.h"
#include "GUI/canvaswindow.h"
#include "singlewidgetabstraction.h"

WindowSingleWidgetTarget::WindowSingleWidgetTarget(CanvasWindow *window) {
    mWindow = window;
}

SingleWidgetAbstraction* WindowSingleWidgetTarget::SWT_getAbstractionForWidget(
        const UpdateFuncs& updateFuncs,
        const int& visiblePartWidgetId) {
    for(const auto& abs : mSWT_allAbstractions) {
        if(abs->getParentVisiblePartWidgetId() == visiblePartWidgetId) {
            return abs.get();
        }
    }
    return SWT_createAbstraction(updateFuncs, visiblePartWidgetId);
}

void WindowSingleWidgetTarget::SWT_addChildrenAbstractions(
        SingleWidgetAbstraction *abstraction,
        const UpdateFuncs &updateFuncs,
        const int& visiblePartWidgetId) {
    mWindow->SWT_addChildrenAbstractions(abstraction, updateFuncs,
                                         visiblePartWidgetId);
}

