#ifndef SINGLEWIDGETABSTRACTION_H
#define SINGLEWIDGETABSTRACTION_H

#include "smartPointers/sharedpointerdefs.h"
class SingleWidgetTarget;

enum SWT_BoxRule : short;
enum SWT_Target : short;
struct SWT_RulesCollection;

class SingleWidgetAbstraction;
template <typename T> using stdfunc = std::function<T>;
typedef stdfunc<void(SingleWidgetAbstraction*, int)> SetAbsFunc;

struct UpdateFuncs {
    stdfunc<void(const SWT_BoxRule &)> fContentUpdateIfIsCurrentRule;
    stdfunc<void(SingleWidgetTarget*, const SWT_Target &)>
        fContentUpdateIfIsCurrentTarget;
    stdfunc<void()> fContentUpdateIfSearchNotEmpty;
    stdfunc<void()> fUpdateParentHeight;
    stdfunc<void()> fUpdateVisibleWidgetsContent;
};

class SingleWidgetAbstraction : public StdSelfRef {
public:
    SingleWidgetAbstraction(const qsptr<SingleWidgetTarget>& target,
                            const UpdateFuncs& updateFuncs,
                            const int &visiblePartId);
    ~SingleWidgetAbstraction();

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

    SingleWidgetTarget *getTarget() const;

    void addChildAbstractionForTarget(SingleWidgetTarget * const target);
    void addChildAbstractionForTargetAt(SingleWidgetTarget * const target,
                                        const int &id);
    void addChildAbstraction(SingleWidgetAbstraction * const abs);
    void addChildAbstractionAt(SingleWidgetAbstraction * const abs,
                               const int &id);

    void removeChildAbstractionForTarget(
            const SingleWidgetTarget * const target);
    void removeChildAbstraction(SingleWidgetAbstraction * const abs);

    void switchContentVisible();

    bool contentVisible();

    int getParentVisiblePartWidgetId() {
        return mVisiblePartWidgetId;
    }

    void scheduleWidgetContentUpdateIfIsCurrentRule(
            const SWT_BoxRule &rule);

    bool isMainTarget() {
        return mIsMainTarget;
    }

    void scheduleWidgetContentUpdateIfSearchNotEmpty();
    void scheduleWidgetContentUpdateIfIsCurrentTarget(
            SingleWidgetTarget * const targetP,
            const SWT_Target &target);

    void setIsMainTarget(const bool &bT) {
        mIsMainTarget = bT;
    }

    SingleWidgetAbstraction *getChildAbstractionForTarget(
            const SingleWidgetTarget * const target);

    void moveChildAbstractionForTargetTo(SingleWidgetTarget * const target,
                                         const int &id);
    void afterContentVisibilityChanged();

    SingleWidgetAbstraction* getParent() const {
        return mParent;
    }

    int getIdInParent() const {
        return mIdInParent;
    }

    int getChildrenCount() const {
        return mChildren.count();
    }

    void removeAlongWithAllChildren_k();
protected:
    void setParent(SingleWidgetAbstraction * const parent) {
        mParent = parent;
    }
    void setIdInParent(const int& id) {
        mIdInParent = id;
    }
private:
    bool mIsMainTarget = false;
    bool mContentVisible = false;
    const int mVisiblePartWidgetId;
    const UpdateFuncs mUpdateFuncs;
    const qptr<SingleWidgetTarget> mTarget;

    void updateChildrenIds(const int& minId, const int& maxId) const {
        for(int i = minId; i <= maxId; i++) {
            mChildren.at(i)->setIdInParent(i);
        }
    }
    QList<stdptr<SingleWidgetAbstraction>> mChildren;
    int mIdInParent = -1;
    stdptr<SingleWidgetAbstraction> mParent;
};

#endif // SINGLEWIDGETABSTRACTION_H
