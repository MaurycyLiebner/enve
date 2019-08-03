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

    void setMainAbstraction(SWT_Abstraction *abs);
    void setCurrentRule(const SWT_BoxRule &rule);
    void setCurrentTarget(SingleWidgetTarget *targetP,
                          const SWT_Target target);
    void setAlwaysShowChildren(const bool alwaysShowChildren);
    void setCurrentSearchText(const QString &text);

    void scheduleContentUpdateIfIsCurrentRule(const SWT_BoxRule &rule);
    bool isCurrentRule(const SWT_BoxRule rule);

    SWT_RulesCollection getCurrentRulesCollection() {
        return mCurrentRulesCollection;
    }

    bool getAlwaysShowChildren() {
        return mAlwaysShowChildren;
    }

    void scheduleContentUpdateIfSearchNotEmpty();
    void scheduleContentUpdateIfIsCurrentTarget(SingleWidgetTarget *targetP,
                                                const SWT_Target target);
    void setCurrentType(const SWT_Type &type);
    int getId() const { return mId; }
    const UpdateFuncs& getUpdateFuncs() const {
        return mUpdateFuncs;
    }
protected:
    void setupUpdateFuncs() {
        mUpdateFuncs.fContentUpdateIfIsCurrentRule =
                [this](const SWT_BoxRule &rule) {
            scheduleContentUpdateIfIsCurrentRule(rule);
        };
        mUpdateFuncs.fContentUpdateIfIsCurrentTarget =
                [this](SingleWidgetTarget* targetP,
                       const SWT_Target target) {
            scheduleContentUpdateIfIsCurrentTarget(targetP, target);
        };
        mUpdateFuncs.fContentUpdateIfSearchNotEmpty = [this]() {
            scheduleContentUpdateIfSearchNotEmpty();
        };
        mUpdateFuncs.fUpdateParentHeight = [this]() {
            planScheduleUpdateParentHeight();
        };
        mUpdateFuncs.fUpdateVisibleWidgetsContent = [this]() {
            planScheduleUpdateVisibleWidgetsContent();
        };
    }

    UpdateFuncs mUpdateFuncs;
    static int mNextId;
    const int mId;
    int mNVisible = 0;
    bool mAlwaysShowChildren = false;
    SWT_RulesCollection mCurrentRulesCollection;
    stdptr<SWT_Abstraction> mMainAbstraction;
};


#endif // SCROLLWIDGETVISIBLEPART_H
