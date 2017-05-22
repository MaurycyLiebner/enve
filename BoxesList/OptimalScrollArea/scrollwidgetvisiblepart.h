#ifndef SCROLLWIDGETVISIBLEPART_H
#define SCROLLWIDGETVISIBLEPART_H

#include <QWidget>
#include <QVBoxLayout>
#include <QtMath>
class ScrollWidget;
class SingleWidgetAbstraction;
class SingleWidget;
class SingleWidgetTarget;

enum SWT_Rule : short;

enum SWT_Target : short {
    SWT_CurrentCanvas,
    SWT_CurrentGroup,
    SWT_All
};

typedef bool (SingleWidgetTarget::*SWT_Checker)();

struct SWT_RulesCollection {
    SWT_RulesCollection();
    SWT_RulesCollection(const SWT_Rule &ruleT,
                        const bool &alwaysShowChildrenT,
                        const SWT_Target &targetT,
                        SWT_Checker typeT,
                        const QString &searchStringT) {
        rule = ruleT;
        alwaysShowChildren = alwaysShowChildrenT;
        target = targetT;
        type = typeT;
        searchString = searchStringT;
    }

    SWT_Rule rule;
    bool alwaysShowChildren;
    SWT_Target target;
    SWT_Checker type;
    QString searchString;
};

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
    void setCurrentTarget(SingleWidgetTarget *targetP,
                          const SWT_Target &target);
    void setAlwaysShowChildren(const bool &alwaysShowChildren);
    void setCurrentSearchText(const QString &text);

    void scheduleContentUpdateIfIsCurrentRule(const SWT_Rule &rule);
    bool isCurrentRule(const SWT_Rule &rule);

    SWT_RulesCollection getCurrentRulesCollection() {
        return mCurrentRulesCollection;
    }

    bool getAlwaysShowChildren() {
        return mAlwaysShowChildren;
    }

    void scheduleContentUpdateIfSearchNotEmpty();
    void scheduleContentUpdateIfIsCurrentTarget(SingleWidgetTarget *targetP,
                                                const SWT_Target &target);
    void setCurrentType(SWT_Checker type);

protected:
    static QList<ScrollWidgetVisiblePart*> mAllInstances;

    SWT_RulesCollection mCurrentRulesCollection;

    bool mVisibleWidgetsContentUpdateScheduled = false;
    bool mParentHeightUpdateScheduled = false;
    bool mAlwaysShowChildren = false;

    ScrollWidget *mParentWidget;

    SingleWidgetAbstraction *mMainAbstraction = NULL;

    QList<SingleWidget*> mSingleWidgets;

    QVBoxLayout *mLayout;
    int mVisibleTop = 0;
    int mVisibleHeight = 0;
};


#endif // SCROLLWIDGETVISIBLEPART_H
