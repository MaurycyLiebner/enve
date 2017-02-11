#ifndef SCROLLWIDGETVISIBLEPART_H
#define SCROLLWIDGETVISIBLEPART_H

#include <QWidget>
#include <QVBoxLayout>
class ScrollWidget;
class SingleWidgetAbstraction;
class SingleWidget;

enum SWT_Rule : short;

class ScrollWidgetVisiblePart :
        public QWidget {
    Q_OBJECT
public:
    ScrollWidgetVisiblePart(ScrollWidget *parent = 0);
    ~ScrollWidgetVisiblePart();

    void setVisibleTop(const int &top);
    void setVisibleHeight(const int &height);

    void scheduledUpdateVisibleWidgetsContent();
    void updateVisibleWidgetsContentIfNeeded();

    void scheduleUpdateParentHeight();
    void updateParentHeightIfNeeded();

    void updateVisibleWidgets();
    void updateVisibleWidgetsContent();

    void setMainAbstraction(SingleWidgetAbstraction *abs);

    void updateParentHeight();

    virtual SingleWidget *createNewSingleWidget();
    void updateWidgetsWidth();

    void callUpdaters();
    static void callAllInstanceUpdaters();

    static void addInstance(ScrollWidgetVisiblePart *instance);
    static void removeInstance(ScrollWidgetVisiblePart *instance);

    void setCurrentRule(const SWT_Rule &rule);

    void scheduleContentUpdateIfIsCurrentRule(const SWT_Rule &rule);
    bool isCurrentRule(const SWT_Rule &rule);

    SWT_Rule getCurrentRule() {
        return mCurrentRule;
    }

protected:
    static QList<ScrollWidgetVisiblePart*> mAllInstances;

    SWT_Rule mCurrentRule;

    bool mVisibleWidgetsContentUpdateScheduled = false;
    bool mParentHeightUpdateScheduled = false;

    ScrollWidget *mParentWidget;

    SingleWidgetAbstraction *mMainAbstraction = NULL;

    QList<SingleWidget*> mSingleWidgets;

    QVBoxLayout *mLayout;
    int mVisibleTop = 0;
    int mVisibleHeight = 0;
};


#endif // SCROLLWIDGETVISIBLEPART_H
