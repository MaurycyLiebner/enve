#ifndef SCROLLWIDGETVISIBLEPART_H
#define SCROLLWIDGETVISIBLEPART_H

#include <QWidget>
#include <QVBoxLayout>
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

enum SWT_Type : short;

struct SWT_RulesCollection {
    SWT_RulesCollection();
    SWT_RulesCollection(const SWT_Rule &ruleT,
                        const bool &alwaysShowChildrenT,
                        const SWT_Target &targetT,
                        const SWT_Type &typeT,
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
    SWT_Type type;
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
    void setCurrentType(const SWT_Type &type);

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
