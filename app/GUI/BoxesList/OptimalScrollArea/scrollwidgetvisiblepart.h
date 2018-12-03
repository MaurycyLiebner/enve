#ifndef SCROLLWIDGETVISIBLEPART_H
#define SCROLLWIDGETVISIBLEPART_H

#include "minimalscrollwidgetvisiblepart.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QtMath>
#include <memory>
#include "smartPointers/sharedpointerdefs.h"
#include "singlewidgettarget.h"
class ScrollWidget;
class SingleWidgetTarget;

enum SWT_Rule : short;

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
    int getId() const { return mId; }
    const UpdateFuncs& getUpdateFuncs() const {
        return mUpdateFuncs;
    }
protected:
    void setupUpdateFuncs() {
        mUpdateFuncs.contentUpdateIfIsCurrentRule =
                [this](const SWT_Rule &rule) {
            scheduleContentUpdateIfIsCurrentRule(rule);
        };
        mUpdateFuncs.contentUpdateIfIsCurrentTarget =
                [this](SingleWidgetTarget* targetP,
                       const SWT_Target &target) {
            scheduleContentUpdateIfIsCurrentTarget(targetP, target);
        };
        mUpdateFuncs.contentUpdateIfSearchNotEmpty = [this]() {
            scheduleContentUpdateIfSearchNotEmpty();
        };
        mUpdateFuncs.updateParentHeight = [this]() {
            scheduleUpdateParentHeight();
        };
        mUpdateFuncs.updateVisibleWidgetsContent = [this]() {
            scheduleUpdateVisibleWidgetsContent();
        };
    }

    UpdateFuncs mUpdateFuncs;
    static int mNextId;
    const int mId;
    bool mAlwaysShowChildren = false;
    SWT_RulesCollection mCurrentRulesCollection;
    stdptr<SingleWidgetAbstraction> mMainAbstraction;
};


#endif // SCROLLWIDGETVISIBLEPART_H
