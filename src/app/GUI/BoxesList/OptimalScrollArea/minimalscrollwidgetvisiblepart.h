#ifndef MINIMALSCROLLWIDGETVISIBLEPART_H
#define MINIMALSCROLLWIDGETVISIBLEPART_H
class MinimalScrollWidget;
class SingleWidget;
#include <QWidget>

class MinimalScrollWidgetVisiblePart : public QWidget {
public:
    MinimalScrollWidgetVisiblePart(MinimalScrollWidget * const parent);

    void setVisibleTop(const int top);
    void setVisibleHeight(const int height);

    void planScheduleUpdateVisibleWidgetsContent();
    void updateVisibleWidgetsContentIfNeeded();

    void planScheduleUpdateParentHeight();
    void updateParentHeightIfNeeded();

    void updateVisibleWidgets();
    virtual void updateVisibleWidgetsContent();

    void updateParentHeight();

    virtual QWidget *createNewSingleWidget() = 0;
    void updateWidgetsWidth();

    void callUpdaters();

    void scheduleContentUpdate();
protected:
    void postEvent();
    bool event(QEvent* event);

    MinimalScrollWidget *mParentWidget;

    QList<QWidget*> mSingleWidgets;

    int mVisibleTop = 0;
    int mVisibleHeight = 0;
private:
    bool mEventSent = false;
    bool mVisibleWidgetsContentUpdateScheduled = false;
    bool mParentHeightUpdateScheduled = false;
};

#endif // MINIMALSCROLLWIDGETVISIBLEPART_H
