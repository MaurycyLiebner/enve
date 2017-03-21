#ifndef SINGLEWIDGETTARGET_H
#define SINGLEWIDGETTARGET_H
#include <QList>
#include <QMenu>
#include <QMimeData>
#include <QFlags>

class SingleWidgetAbstraction;
class ScrollWidgetVisiblePart;

enum SWT_Type : short {
    SWT_BoundingBox = 1,
    SWT_BoxesGroup = 2,
    SWT_PixmapEffect = 4,
    SWT_QrealAnimator = 8,
    SWT_ComplexAnimator = 16,
    SWT_ColorAnimator = 32,
    SWT_Canvas = 64,
    SWT_BoxTarget = 128,
    SWT_BoolProperty = 256,
    SWT_SingleSound = 512
};

Q_DECLARE_FLAGS(SWT_Types, SWT_Type)
Q_DECLARE_OPERATORS_FOR_FLAGS(SWT_Types)

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

enum SWT_Target : short;

struct SWT_RulesCollection;

class SingleWidgetTarget {
public:
    SingleWidgetTarget();
    virtual ~SingleWidgetTarget();

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

    virtual bool SWT_satisfiesRule(const SWT_RulesCollection &rules,
                                   const bool &parentSatisfies) {
        Q_UNUSED(rules);
        return parentSatisfies;
    }

    void SWT_scheduleWidgetsContentUpdateWithRule(
            const SWT_Rule &rule);

    virtual bool SWT_visibleOnlyIfParentDescendant() {
        return true;
    }

    void SWT_scheduleWidgetsContentUpdateWithSearchNotEmpty();
    void SWT_scheduleWidgetsContentUpdateWithTarget(SingleWidgetTarget *targetP,
                                                    const SWT_Target &target);

    void SWT_moveChildAbstractionForTargetToInAll(SingleWidgetTarget *target,
                                                  const int &id);

    virtual void SWT_addToContextMenu(QMenu *menu) {
        Q_UNUSED(menu);
    }
    virtual bool SWT_handleContextMenuActionSelected(QAction *selectedAction) {
        Q_UNUSED(selectedAction);
        return false;
    }

    virtual QMimeData *SWT_createMimeData() {
        return NULL;
    }
protected:
    QList<SingleWidgetAbstraction*> mSWT_allAbstractions;

    void SWT_addChildAbstractionForTargetToAll(
            SingleWidgetTarget *target);
    void SWT_removeChildAbstractionForTargetFromAll(
            SingleWidgetTarget *target);
};

#endif // SINGLEWIDGETTARGET_H
