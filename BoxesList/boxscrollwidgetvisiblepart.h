#ifndef BOXSCROLLWIDGETVISIBLEPART_H
#define BOXSCROLLWIDGETVISIBLEPART_H

#include <QWidget>
#include "OptimalScrollArea/scrollwidgetvisiblepart.h"

class BoxScrollWidgetVisiblePart : public ScrollWidgetVisiblePart
{
    Q_OBJECT
public:
    explicit BoxScrollWidgetVisiblePart(ScrollWidget *parent = 0);

    SingleWidget *createNewSingleWidget();
signals:

public slots:
};

#endif // BOXSCROLLWIDGETVISIBLEPART_H
