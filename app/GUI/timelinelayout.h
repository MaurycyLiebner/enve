#ifndef TIMELINELAYOUT_H
#define TIMELINELAYOUT_H
#include "layoutcollection.h"
class TimelineWrapper;
class ChangeWidthWidget;
class BoxesListAnimationDockWidget;

class TimelineLayout : public QWidget {
public:
    TimelineLayout(QWidget * const parent = nullptr);

    SceneBaseStackItem* extract();
    void setCurrent(SceneBaseStackItem* const item);
private:
    void setWidget(QWidget* const wid);
    void reset();

    SceneBaseStackItem* mBaseStack = nullptr;
};

#endif // TIMELINELAYOUT_H
