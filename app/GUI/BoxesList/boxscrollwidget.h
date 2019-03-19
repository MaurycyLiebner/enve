#ifndef BOXSCROLLWIDGET_H
#define BOXSCROLLWIDGET_H

#include "OptimalScrollArea/scrollwidget.h"
#include "GUI/keyfocustarget.h"
class BoxScrollWidgetVisiblePart;
class ScrollArea;
class WindowSingleWidgetTarget;

class BoxScrollWidget : public ScrollWidget {
    Q_OBJECT
public:
    explicit BoxScrollWidget(WindowSingleWidgetTarget * const target,
                             ScrollArea * const parent = nullptr);
    ~BoxScrollWidget();
    BoxScrollWidgetVisiblePart *getVisiblePartWidget();
    //    void updateAbstraction();
protected:
    stdptr<SingleWidgetAbstraction> mCoreAbs;
    void createVisiblePartWidget();
};

#endif // BOXSCROLLWIDGET_H
