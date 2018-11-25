#ifndef SINGLEWIDGETABSTRACTION_H
#define SINGLEWIDGETABSTRACTION_H

#include "selfref.h"
#include "sharedpointerdefs.h"
#include <QWidget>
class SingleWidgetTarget;
class SingleWidget;
class ScrollWidgetVisiblePart;

enum SWT_Rule : short;
enum SWT_Target : short;
struct SWT_RulesCollection;

class SingleWidgetAbstraction : public StdSelfRef {
public:
    SingleWidgetAbstraction(const SingleWidgetTargetQSPtr& target,
                            ScrollWidgetVisiblePart *visiblePart);
    virtual ~SingleWidgetAbstraction();

    bool getAbstractions(const int &minY, const int &maxY,
                         int& currY, int currX,
                         QList<SingleWidgetAbstractionSPtr>& abstractions,
                         const SWT_RulesCollection &rules,
                         const bool &parentSatisfiesRule,
                         const bool &parentMainTarget);
    bool setSingleWidgetAbstractions(const int &minY, const int &maxY,
                                     int *currY, int currX,
                                     QList<QWidget *> *widgets,
                                     int *currentWidgetId,
                                     const SWT_RulesCollection &rules,
                                     const bool &parentSatisfiesRule,
                                     const bool &parentMainTarget);

    int getHeight(const SWT_RulesCollection &rules,
                  const bool &parentSatisfiesRule,
                  const bool &parentMainTarget);

    void setContentVisible(const bool &bT);

    SingleWidgetTarget *getTarget();

    void addChildAbstractionForTarget(SingleWidgetTarget *target);
    void addChildAbstractionForTargetAt(
            SingleWidgetTarget *target,
            const int &id);
    void addChildAbstraction(const SingleWidgetAbstractionSPtr& abs);
    void addChildAbstractionAt(const SingleWidgetAbstractionSPtr& abs,
                               const int &id);

    void removeChildAbstractionForTarget(SingleWidgetTarget *target);
    void removeChildAbstraction(const SingleWidgetAbstractionSPtr& abs);

    void switchContentVisible();

    bool contentVisible();

    ScrollWidgetVisiblePart *getParentVisiblePartWidget() {
        return mVisiblePartWidget;
    }

    void scheduleWidgetContentUpdateIfIsCurrentRule(
            const SWT_Rule &rule);

    bool isMainTarget() {
        return mIsMainTarget;
    }

    void scheduleWidgetContentUpdateIfSearchNotEmpty();
    void scheduleWidgetContentUpdateIfIsCurrentTarget(
            SingleWidgetTarget *targetP,
            const SWT_Target &target);

    void setIsMainTarget(const bool &bT) {
        mIsMainTarget = bT;
    }

    SingleWidgetAbstraction *getChildAbstractionForTarget(
            SingleWidgetTarget *target);

    void moveChildAbstractionForTargetTo(SingleWidgetTarget *target,
                                         const int &id);
    void afterContentVisibilityChanged();
private:
    ScrollWidgetVisiblePart *mVisiblePartWidget;
    bool mIsMainTarget = false;
    bool mContentVisible = false;
    SingleWidgetTarget* mTarget = nullptr;

    QList<SingleWidgetAbstractionSPtr> mChildren;
};

#endif // SINGLEWIDGETABSTRACTION_H
