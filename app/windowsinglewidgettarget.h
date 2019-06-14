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

    void SWT_setupAbstraction(SWT_Abstraction *abstraction,
                                     const UpdateFuncs &updateFuncs,
                                     const int visiblePartWidgetId);
protected:
    WindowSingleWidgetTarget(CanvasWindow * const window);

    CanvasWindow * const mWindow;
};

#endif // WINDOWSINGLEWIDGETTARGET_H
