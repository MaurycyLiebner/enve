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

class ScrollWidgetVisiblePart :
        public MinimalScrollWidgetVisiblePart {
    Q_OBJECT
public:
    ScrollWidgetVisiblePart(ScrollWidget * const parent);

    virtual QWidget *createNewSingleWidget();
    void updateVisibleWidgetsContent();
    void callUpdaters();

    void setMainAbstraction(SingleWidgetAbstraction *abs);
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
        mUpdateFuncs.fContentUpdateIfIsCurrentRule =
                [this](const SWT_Rule &rule) {
            scheduleContentUpdateIfIsCurrentRule(rule);
        };
        mUpdateFuncs.fContentUpdateIfIsCurrentTarget =
                [this](SingleWidgetTarget* targetP,
                       const SWT_Target &target) {
            scheduleContentUpdateIfIsCurrentTarget(targetP, target);
        };
        mUpdateFuncs.fContentUpdateIfSearchNotEmpty = [this]() {
            scheduleContentUpdateIfSearchNotEmpty();
        };
        mUpdateFuncs.fUpdateParentHeight = [this]() {
            scheduleUpdateParentHeight();
        };
        mUpdateFuncs.fUpdateVisibleWidgetsContent = [this]() {
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
