#ifndef SINGLEWIDGETABSTRACTION_H
#define SINGLEWIDGETABSTRACTION_H

#include "smartPointers/sharedpointerdefs.h"
class SingleWidgetTarget;

enum SWT_Rule : short;
enum SWT_Target : short;
struct SWT_RulesCollection;

class SingleWidgetAbstraction;
template <typename T> using stdfunc = std::function<T>;
typedef std::function<void(SingleWidgetAbstraction*, int)> SetAbsFunc;

struct UpdateFuncs {
    stdfunc<void(const SWT_Rule &)> contentUpdateIfIsCurrentRule;
    stdfunc<void(SingleWidgetTarget*, const SWT_Target &)>
        contentUpdateIfIsCurrentTarget;
    stdfunc<void()> contentUpdateIfSearchNotEmpty;
    stdfunc<void()> updateParentHeight;
    stdfunc<void()> updateVisibleWidgetsContent;
};

class SingleWidgetAbstraction : public StdSelfRef {
public:
    SingleWidgetAbstraction(const qsptr<SingleWidgetTarget>& target,
                            const UpdateFuncs& updateFuncs,
                            const int &visiblePartId);
    virtual ~SingleWidgetAbstraction();

    bool getAbstractions(const int &minY, const int &maxY,
                         int& currY, int currX,
                         const int &swtHeight,
                         QList<SingleWidgetAbstraction*>& abstractions,
                         const SWT_RulesCollection &rules,
                         const bool &parentSatisfiesRule,
                         const bool &parentMainTarget);
    bool setSingleWidgetAbstractions(const int &minY, const int &maxY,
                                     int &currY, int currX,
                                     const int &swtHeight,
                                     const SetAbsFunc& setAbsFunc,
                                     const SWT_RulesCollection &rules,
                                     const bool &parentSatisfiesRule,
                                     const bool &parentMainTarget);

    int getHeight(const SWT_RulesCollection &rules,
                  const bool &parentSatisfiesRule,
                  const bool &parentMainTarget,
                  const int& swtHeight);

    void setContentVisible(const bool &bT);

    SingleWidgetTarget *getTarget();

    void addChildAbstractionForTarget(SingleWidgetTarget *target);
    void addChildAbstractionForTargetAt(SingleWidgetTarget *target,
                                        const int &id);
    void addChildAbstraction(SingleWidgetAbstraction *abs);
    void addChildAbstractionAt(SingleWidgetAbstraction *abs,
                               const int &id);

    void removeChildAbstractionForTarget(SingleWidgetTarget *target);
    void removeChildAbstraction(SingleWidgetAbstraction* abs);

    void switchContentVisible();

    bool contentVisible();

    int getParentVisiblePartWidgetId() {
        return mVisiblePartWidgetId;
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
    bool mIsMainTarget = false;
    bool mContentVisible = false;
    const int mVisiblePartWidgetId;
    const UpdateFuncs mUpdateFuncs;
    const qptr<SingleWidgetTarget> mTarget;

    QList<stdptr<SingleWidgetAbstraction>> mChildren;
};

#endif // SINGLEWIDGETABSTRACTION_H
