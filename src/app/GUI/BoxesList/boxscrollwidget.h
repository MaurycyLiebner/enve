#ifndef BOXSCROLLWIDGET_H
#define BOXSCROLLWIDGET_H

#include "OptimalScrollArea/scrollwidget.h"
#include "GUI/keyfocustarget.h"
class BoxScroller;
class ScrollArea;
class WindowSingleWidgetTarget;
class Document;
class Canvas;

class BoxScrollWidget : public ScrollWidget {
    Q_OBJECT
public:
    explicit BoxScrollWidget(Document& document,
                             ScrollArea * const parent = nullptr);
    ~BoxScrollWidget();
    BoxScroller *getVisiblePartWidget();
    void setCurrentScene(Canvas* const scene);
    //    void updateAbstraction();
protected:
    stdptr<SWT_Abstraction> mCoreAbs;
    void createVisiblePartWidget();
};

#endif // BOXSCROLLWIDGET_H
