#ifndef SINGLEWIDGETABSTRACTION_H
#define SINGLEWIDGETABSTRACTION_H

#include "smartPointers/sharedpointerdefs.h"
class SingleWidgetTarget;

enum SWT_BoxRule : short;
enum SWT_Target : short;
struct SWT_RulesCollection;

class SWT_Abstraction;
template <typename T> using stdfunc = std::function<T>;
typedef stdfunc<void(SWT_Abstraction*, int)> SetAbsFunc;

struct UpdateFuncs {
    stdfunc<void(const SWT_BoxRule &)> fContentUpdateIfIsCurrentRule;
    stdfunc<void(SingleWidgetTarget*, const SWT_Target &)>
        fContentUpdateIfIsCurrentTarget;
    stdfunc<void()> fContentUpdateIfSearchNotEmpty;
    stdfunc<void()> fUpdateParentHeight;
    stdfunc<void()> fUpdateVisibleWidgetsContent;
};

class SWT_Abstraction : public StdSelfRef {
public:
    SWT_Abstraction(SingleWidgetTarget * const target,
                            const UpdateFuncs& updateFuncs,
                            const int visiblePartId);

    bool setAbstractions(const int minY, const int maxY,
                         int &currY, int currX,
                         const int swtHeight,
                         const SetAbsFunc& setAbsFunc,
                         const SWT_RulesCollection &rules,
                         const bool parentSatisfiesRule,
                         const bool parentMainTarget);

    int getHeight(const SWT_RulesCollection &rules,
                  const bool parentSatisfiesRule,
                  const bool parentMainTarget,
                  const int swtHeight);

    void setContentVisible(const bool bT);

    SingleWidgetTarget *getTarget() const;

    void addChild(SingleWidgetTarget * const target);
    void addChildAt(SingleWidgetTarget * const target,
                                        const int id);
    void addChildAbstraction(SWT_Abstraction * const abs);
    void addChildAbstractionAt(SWT_Abstraction * const abs,
                               const int id);

    void removeChild(
            const SingleWidgetTarget * const target);
    void removeChild(SWT_Abstraction * const abs);

    void switchContentVisible();

    bool contentVisible();

    int getParentVisiblePartWidgetId() {
        return mVisiblePartWidgetId;
    }

    void scheduleContentUpdate(const SWT_BoxRule &rule);

    bool isMainTarget() {
        return mIsMainTarget;
    }

    void scheduleSearchContentUpdate();
    void scheduleContentUpdate(
            SingleWidgetTarget * const targetP,
            const SWT_Target &target);

    void setIsMainTarget(const bool bT) {
        mIsMainTarget = bT;
    }

    SWT_Abstraction *getChildAbsFor(
            const SingleWidgetTarget * const target);

    void moveChildTo(SingleWidgetTarget * const target,
                                         const int id);
    void afterContentVisibilityChanged();

    SWT_Abstraction* getParent() const {
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
    void setParent(SWT_Abstraction * const parent) {
        mParent = parent;
    }
    void setIdInParent(const int id) {
        mIdInParent = id;
    }
private:
    bool mIsMainTarget = false;
    bool mContentVisible = false;
    const int mVisiblePartWidgetId;
    const UpdateFuncs mUpdateFuncs;
    SingleWidgetTarget * const mTarget_k;

    void updateChildrenIds(const int minId, const int maxId) const {
        for(int i = minId; i <= maxId; i++) {
            mChildren.at(i)->setIdInParent(i);
        }
    }
    QList<stdptr<SWT_Abstraction>> mChildren;
    int mIdInParent = -1;
    stdptr<SWT_Abstraction> mParent;
};

#endif // SINGLEWIDGETABSTRACTION_H
