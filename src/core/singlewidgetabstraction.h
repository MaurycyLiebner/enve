#ifndef SINGLEWIDGETABSTRACTION_H
#define SINGLEWIDGETABSTRACTION_H

#include "smartPointers/ememory.h"
class SingleWidgetTarget;

enum SWT_BoxRule : short;
enum SWT_Target : short;
struct SWT_RulesCollection;

class SWT_Abstraction;
template <typename T> using stdfunc = std::function<T>;
typedef stdfunc<void(SWT_Abstraction*, int)> SetAbsFunc;

struct UpdateFuncs {
    stdfunc<void(const SWT_BoxRule)> fContentUpdateIfIsCurrentRule;
    stdfunc<void(SingleWidgetTarget*, const SWT_Target)>
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

    int updateHeight(const SWT_RulesCollection &rules,
                     const bool parentSatisfiesRule,
                     const bool parentMainTarget,
                     const int swtHeight);
    int getHeight() const { return mHeight; }

    void setContentVisible(const bool bT);

    SingleWidgetTarget *getTarget() const;

    void addChild(SingleWidgetTarget * const target);
    void addChildAt(SingleWidgetTarget * const target, const int id);
    void addChildAbstraction(SWT_Abstraction * const abs);
    void addChildAbstractionAt(SWT_Abstraction * const abs, const int id);

    void removeChild(const SingleWidgetTarget * const target);
    void removeChild(SWT_Abstraction * const abs);

    void switchContentVisible();

    bool contentVisible();

    int getParentVisiblePartWidgetId() {
        return mVisiblePartWidgetId;
    }

    void scheduleContentUpdate(const SWT_BoxRule rule);
    void scheduleSearchContentUpdate();

    bool isMainTarget() { return mIsMainTarget; }

    void setIsMainTarget(const bool bT) {
        mIsMainTarget = bT;
    }

    SWT_Abstraction *getChildAbsFor(const SingleWidgetTarget * const target);

    void moveChildTo(SingleWidgetTarget * const target, const int id);
    void afterContentVisibilityChanged();

    SWT_Abstraction* getParent() const {
        return mParent;
    }

    int getIdInParent() const {
        return mIdInParent;
    }

    int childrenCount() const {
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
    void updateChildrenIds(const int minId, const int maxId) const {
        for(int i = minId; i <= maxId; i++) {
            mChildren.at(i)->setIdInParent(i);
        }
    }

    bool mIsMainTarget = false;
    bool mContentVisible = false;
    int mHeight = 0;
    const int mVisiblePartWidgetId;
    const UpdateFuncs mUpdateFuncs;
    SingleWidgetTarget * const mTarget_k;

    QList<stdptr<SWT_Abstraction>> mChildren;
    int mIdInParent = -1;
    stdptr<SWT_Abstraction> mParent;
};

#endif // SINGLEWIDGETABSTRACTION_H
