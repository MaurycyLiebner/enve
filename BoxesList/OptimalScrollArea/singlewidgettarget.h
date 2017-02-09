#ifndef SINGLEWIDGETTARGET_H
#define SINGLEWIDGETTARGET_H
#include <QList>
class SingleWidgetAbstraction;
class ScrollWidgetVisiblePart;

enum SingleWidgetTargetType {
    SWT_BoundingBox,
    SWT_BoxesGroup,
    SWT_QrealAnimator,
    SWT_ComplexAnimator
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

    virtual SingleWidgetTargetType SWT_getType() = 0;

protected:
    QList<SingleWidgetAbstraction*> mSWT_allAbstractions;

    void SWT_addChildAbstractionForTargetToAll(
            SingleWidgetTarget *target);
    void SWT_removeChildAbstractionForTargetFromAll(
            SingleWidgetTarget *target);
};

#endif // SINGLEWIDGETTARGET_H
