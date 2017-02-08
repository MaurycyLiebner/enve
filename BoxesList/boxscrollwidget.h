#ifndef BOXSCROLLWIDGET_H
#define BOXSCROLLWIDGET_H

#include "OptimalScrollArea/scrollwidget.h"
#include "boxscrollwidgetvisiblepart.h"

class BoxScrollWidget : public ScrollWidget
{
    Q_OBJECT
public:
    explicit BoxScrollWidget(QWidget *parent = 0);

protected:
    void createVisiblePartWidget();
};

#endif // BOXSCROLLWIDGET_H
