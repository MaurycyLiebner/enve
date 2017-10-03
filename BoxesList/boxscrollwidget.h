#ifndef BOXSCROLLWIDGET_H
#define BOXSCROLLWIDGET_H

#include "OptimalScrollArea/scrollwidget.h"
#include "keyfocustarget.h"
class BoxScrollWidgetVisiblePart;
class ScrollArea;

class BoxScrollWidget : public ScrollWidget {
    Q_OBJECT
public:
    explicit BoxScrollWidget(ScrollArea *parent = 0);
    BoxScrollWidgetVisiblePart *getVisiblePartWidget();
    //    void updateAbstraction();
protected:
    void createVisiblePartWidget();
};

#endif // BOXSCROLLWIDGET_H
