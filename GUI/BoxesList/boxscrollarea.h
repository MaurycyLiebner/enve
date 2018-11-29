#ifndef BOXSCROLLAREA_H
#define BOXSCROLLAREA_H
#include "OptimalScrollArea/scrollarea.h"

class BoxScrollArea : public ScrollArea
{
public:
    BoxScrollArea(QWidget *parent = nullptr);
protected:
    void focusInEvent(QFocusEvent *);
};

#endif // BOXSCROLLAREA_H
