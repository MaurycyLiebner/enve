#ifndef WINDOWSINGLEWIDGETTARGET_H
#define WINDOWSINGLEWIDGETTARGET_H
class CanvasWindow;
#include "singlewidgettarget.h"

class WindowSingleWidgetTarget : public SingleWidgetTarget {
    friend class SelfRef;
public:
    CanvasWindow* getCanvasWindow() const {
        return mWindow;
    }
    SingleWidgetAbstraction *SWT_getAbstractionForWidget(
            ScrollWidgetVisiblePart *visiblePartWidget);
    void SWT_addChildrenAbstractions(
            SingleWidgetAbstraction *abstraction,
            ScrollWidgetVisiblePart *visiblePartWidget);
protected:
    WindowSingleWidgetTarget(CanvasWindow *window);

    CanvasWindow *mWindow = nullptr;
};

#endif // WINDOWSINGLEWIDGETTARGET_H
