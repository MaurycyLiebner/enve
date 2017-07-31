#ifndef MINIMALSCROLLWIDGETVISIBLEPART_H
#define MINIMALSCROLLWIDGETVISIBLEPART_H
class MinimalScrollWidget;
class SingleWidget;
#include <QList>

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

    virtual SingleWidget *createNewSingleWidget();
    void updateWidgetsWidth();

    void callUpdaters();

    static void callAllInstanceUpdaters();
    static void addInstance(MinimalScrollWidgetVisiblePart *instance);
    static void removeInstance(MinimalScrollWidgetVisiblePart *instance);

    void setAlwaysShowChildren(const bool &alwaysShowChildren);
    void setCurrentSearchText(const QString &text);

    void scheduleContentUpdateIfSearchNotEmpty();

    void scheduleContentUpdate();
protected:
    static QList<MinimalScrollWidgetVisiblePart*> mAllInstances;

    bool mVisibleWidgetsContentUpdateScheduled = false;
    bool mParentHeightUpdateScheduled = false;

    MinimalScrollWidget *mParentWidget;

    QList<SingleWidget*> mSingleWidgets;

    int mVisibleTop = 0;
    int mVisibleHeight = 0;
};

#endif // MINIMALSCROLLWIDGETVISIBLEPART_H
