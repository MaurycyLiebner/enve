#ifndef SINGLEWIDGETTARGET_H
#define SINGLEWIDGETTARGET_H
#include <QList>
class SingleWidgetAbstraction;
class ScrollWidgetVisiblePart;

enum SWT_Type : short {
    SWT_BoundingBox,
    SWT_BoxesGroup,
    SWT_QrealAnimator,
    SWT_ComplexAnimator,
    SWT_Canvas
};

enum SWT_Rule : short {
    SWT_NoRule,
    SWT_Selected,
    SWT_Visible,
    SWT_Invisible,
    SWT_Locked,
    SWT_Unlocked,
    SWT_Animated,
    SWT_NotAnimated
};

class SingleWidgetTarget {
public:
    SingleWidgetTarget();
    ~SingleWidgetTarget() {}

    SingleWidgetAbstraction *SWT_createAbstraction(
            ScrollWidgetVisiblePart *visiblePartWidget);
    void SWT_removeAbstractionFromList(SingleWidgetAbstraction *abs);

    virtual void SWT_addChildrenAbstractions(
            SingleWidgetAbstraction *,
            ScrollWidgetVisiblePart *) {}

    virtual SWT_Type SWT_getType() = 0;

    virtual SingleWidgetAbstraction* SWT_getAbstractionForWidget(
            ScrollWidgetVisiblePart *visiblePartWidget) {
        return SWT_createAbstraction(visiblePartWidget);
    }
    void SWT_addChildAbstractionForTargetToAllAt(
            SingleWidgetTarget *target, const int &id);

    virtual bool SWT_satisfiesRule(const bool &parentSatisfies,
                                   const SWT_Rule &rule) {
        Q_UNUSED(rule);
        return parentSatisfies;
    }

    void SWT_scheduleWidgetsContentUpdateWithRule(
            const SWT_Rule &rule);

protected:
    QList<SingleWidgetAbstraction*> mSWT_allAbstractions;

    void SWT_addChildAbstractionForTargetToAll(
            SingleWidgetTarget *target);
    void SWT_removeChildAbstractionForTargetFromAll(
            SingleWidgetTarget *target);
};

#endif // SINGLEWIDGETTARGET_H
