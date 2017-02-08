#ifndef SINGLEWIDGETTARGET_H
#define SINGLEWIDGETTARGET_H

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

    virtual void SWT_addChildrenAbstractions(
            SingleWidgetAbstraction *abstraction,
            ScrollWidgetVisiblePart *visiblePartWidget);

    virtual SingleWidgetTargetType SWT_getType() = 0;
protected:
};

#endif // SINGLEWIDGETTARGET_H
