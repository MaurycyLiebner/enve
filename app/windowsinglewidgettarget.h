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
            const UpdateFuncs &updateFuncs,
            const int& visiblePartWidgetId);
    void SWT_addChildrenAbstractions(SingleWidgetAbstraction *abstraction,
                                     const UpdateFuncs &updateFuncs,
                                     const int& visiblePartWidgetId);
protected:
    WindowSingleWidgetTarget(CanvasWindow *window);

    CanvasWindow *mWindow = nullptr;
};

#endif // WINDOWSINGLEWIDGETTARGET_H
