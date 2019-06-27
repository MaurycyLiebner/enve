#ifndef TIMELINELAYOUT_H
#define TIMELINELAYOUT_H
#include "layoutcollection.h"
class TimelineWrapper;
class ChangeWidthWidget;
class BoxesListAnimationDockWidget;

class TimelineLayout : public QWidget {
public:
    TimelineLayout(Document& document, QWidget * const parent = nullptr);

    SceneBaseStackItem::cUPtr extract();
    void setCurrent(const SceneBaseStackItem* const item);
private:
    void setWidget(QWidget* const wid);
    void reset(TimelineWrapper ** const cwwP = nullptr);

    Document& mDocument;
    SceneBaseStackItem::cUPtr mBaseStack;
};

#endif // TIMELINELAYOUT_H
