#ifndef BOXSCROLLWIDGET_H
#define BOXSCROLLWIDGET_H

#include "OptimalScrollArea/scrollwidget.h"
#include "GUI/keyfocustarget.h"
class BoxScrollWidgetVisiblePart;
class ScrollArea;

class BoxScrollWidget : public ScrollWidget {
    Q_OBJECT
public:
    explicit BoxScrollWidget(ScrollArea *parent = nullptr);
    BoxScrollWidgetVisiblePart *getVisiblePartWidget();
    //    void updateAbstraction();
protected:
    void createVisiblePartWidget();
};

#endif // BOXSCROLLWIDGET_H
