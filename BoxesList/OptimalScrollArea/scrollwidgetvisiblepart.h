#ifndef SCROLLWIDGETVISIBLEPART_H
#define SCROLLWIDGETVISIBLEPART_H

#include "minimalscrollwidgetvisiblepart.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QtMath>
#include <memory>
#include "sharedpointerdefs.h"
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
        public MinimalScrollWidgetVisiblePart {
    Q_OBJECT
public:
    ScrollWidgetVisiblePart(ScrollWidget *parent = nullptr);
    ~ScrollWidgetVisiblePart();

    void updateVisibleWidgetsContent();

    void setMainAbstraction(SingleWidgetAbstraction *abs);

    virtual QWidget *createNewSingleWidget();

    void callUpdaters();

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
    bool mAlwaysShowChildren = false;
    SWT_RulesCollection mCurrentRulesCollection;
    SingleWidgetAbstractionPtr mMainAbstraction;
};


#endif // SCROLLWIDGETVISIBLEPART_H
