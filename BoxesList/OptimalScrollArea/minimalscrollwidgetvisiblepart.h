#ifndef MINIMALSCROLLWIDGETVISIBLEPART_H
#define MINIMALSCROLLWIDGETVISIBLEPART_H
class MinimalScrollWidget;
class SingleWidget;
#include <QWidget>

class MinimalScrollWidgetVisiblePart :
        public QWidget {
    Q_OBJECT
public:
    MinimalScrollWidgetVisiblePart(MinimalScrollWidget *parent = 0);
    virtual ~MinimalScrollWidgetVisiblePart();

    void setVisibleTop(const int &top);
    void setVisibleHeight(const int &height);

    void scheduledUpdateVisibleWidgetsContent();
    void updateVisibleWidgetsContentIfNeeded();

    void scheduleUpdateParentHeight();
    void updateParentHeightIfNeeded();

    void updateVisibleWidgets();
    virtual void updateVisibleWidgetsContent();

    void updateParentHeight();

    virtual QWidget *createNewSingleWidget() = 0;
    void updateWidgetsWidth();

    virtual void callUpdaters();

    static void callAllInstanceUpdaters();
    static void addInstance(MinimalScrollWidgetVisiblePart *instance);
    static void removeInstance(MinimalScrollWidgetVisiblePart *instance);

    void scheduleContentUpdate();
protected:
    static QList<MinimalScrollWidgetVisiblePart*> mAllInstances;

    bool mVisibleWidgetsContentUpdateScheduled = false;
    bool mParentHeightUpdateScheduled = false;

    MinimalScrollWidget *mParentWidget;

    QList<QWidget*> mSingleWidgets;

    int mVisibleTop = 0;
    int mVisibleHeight = 0;
};

#endif // MINIMALSCROLLWIDGETVISIBLEPART_H
