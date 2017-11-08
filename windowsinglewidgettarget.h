#ifndef WINDOWSINGLEWIDGETTARGET_H
#define WINDOWSINGLEWIDGETTARGET_H
class CanvasWindow;
#include "BoxesList/OptimalScrollArea/singlewidgettarget.h"

class WindowSingleWidgetTarget : public SingleWidgetTarget {
public:
    WindowSingleWidgetTarget(CanvasWindow *window);

    SingleWidgetAbstraction *SWT_getAbstractionForWidget(
            ScrollWidgetVisiblePart *visiblePartWidget);
    void SWT_addChildrenAbstractions(
            SingleWidgetAbstraction *abstraction,
            ScrollWidgetVisiblePart *visiblePartWidget);
protected:
    CanvasWindow *mWindow = NULL;
};

#endif // WINDOWSINGLEWIDGETTARGET_H
