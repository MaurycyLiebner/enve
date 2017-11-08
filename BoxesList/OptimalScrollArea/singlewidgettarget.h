#ifndef SINGLEWIDGETTARGET_H
#define SINGLEWIDGETTARGET_H
#include <QList>
#include <QMenu>
#include <QMimeData>
#include <QFlags>
#include <memory>
#include "selfref.h"

class SingleWidgetAbstraction;
class ScrollWidgetVisiblePart;

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

class SingleWidgetTarget : public SelfRef {
public:
    SingleWidgetTarget();
    virtual ~SingleWidgetTarget();

    void SWT_addChildAbstractionForTargetToAll(
            SingleWidgetTarget *target);
    void SWT_removeChildAbstractionForTargetFromAll(
            SingleWidgetTarget *target);

    SingleWidgetAbstraction *SWT_createAbstraction(
            ScrollWidgetVisiblePart *visiblePartWidget);
    void SWT_removeAbstractionFromList(SingleWidgetAbstraction *abs);

    virtual void SWT_addChildrenAbstractions(
            SingleWidgetAbstraction *,
            ScrollWidgetVisiblePart *) {}

    // Animators
    virtual bool SWT_isAnimator() { return false; }
    virtual bool SWT_isBoolAnimator() { return false; }
    virtual bool SWT_isColorAnimator() { return false; }
    virtual bool SWT_isComplexAnimator() { return false; }
    virtual bool SWT_isFakeComplexAnimator() { return false; }
    virtual bool SWT_isPixmapEffectAnimators() { return false; }
    virtual bool SWT_isPathEffectAnimators() { return false; }
    virtual bool SWT_isPixmapEffect() { return false; }
    virtual bool SWT_isPathEffect() { return false; }
    virtual bool SWT_isIntAnimator() { return false; }
    virtual bool SWT_isGradient() { return false; }
    virtual bool SWT_isPaintSettings() { return false; }
    virtual bool SWT_isStrokeSettings() { return false; }
    virtual bool SWT_isPathAnimator() { return false; }
    virtual bool SWT_isQPointFAnimator() { return false; }
    virtual bool SWT_isQrealAnimator() { return false; }
    virtual bool SWT_isQStringAnimator() { return false; }
    virtual bool SWT_isBasicTransformAnimator() { return false; }
    virtual bool SWT_isBoxTransformAnimator() { return false; }
    virtual bool SWT_isVectorPathAnimator() { return false; }
    virtual bool SWT_isAnimatedSurface() { return false; }
    // Boxes
    virtual bool SWT_isCanvas() { return false; }
    virtual bool SWT_isAnimationBox() { return false; }
    virtual bool SWT_isBoundingBox() { return false; }
    virtual bool SWT_isBoxesGroup() { return false; }
    virtual bool SWT_isCircle() { return false; }
    virtual bool SWT_isImageBox() { return false; }
    virtual bool SWT_isImageSequenceBox() { return false; }
    virtual bool SWT_isLinkBox() { return false; }
    virtual bool SWT_isParticleBox() { return false; }
    virtual bool SWT_isPathBox() { return false; }
    virtual bool SWT_isRectangle() { return false; }
    virtual bool SWT_isTextBox() { return false; }
    virtual bool SWT_isVectorPath() { return false; }
    virtual bool SWT_isVideoBox() { return false; }
    virtual bool SWT_isPaintBox() { return false; }
    // Properties
    virtual bool SWT_isBoolProperty() { return false; }
    virtual bool SWT_isComboBoxProperty() { return false; }
    virtual bool SWT_isBoxTargetProperty() { return false; }
    virtual bool SWT_isProperty() { return false; }
    virtual bool SWT_isIntProperty() { return false; }
    // Sound
    virtual bool SWT_isSingleSound() { return false; }

    virtual SingleWidgetAbstraction* SWT_getAbstractionForWidget(
            ScrollWidgetVisiblePart *visiblePartWidget) {
        return SWT_createAbstraction(visiblePartWidget);
    }
    void SWT_addChildAbstractionForTargetToAllAt(
            SingleWidgetTarget *target, const int &id);

    virtual bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                                     const bool &parentSatisfies,
                                     const bool &parentMainTarget) {
        Q_UNUSED(rules);
        return parentSatisfies && !parentMainTarget;
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

    virtual void SWT_clearAll();
    bool SWT_isVisible() {
        return SWT_mVisible;
    }

    void SWT_hide() {
        SWT_mVisible = false;
    }

    void SWT_show() {
        SWT_mVisible = true;
    }
protected:
    bool SWT_mVisible = true;
    QList<std::shared_ptr<SingleWidgetAbstraction> > mSWT_allAbstractions;
};

typedef bool (SingleWidgetTarget::*SWT_Checker)();

struct SWT_TargetTypes {
    bool isTargeted(SingleWidgetTarget *target) const {
        Q_FOREACH(SWT_Checker func, targetsFunctionList) {
            if((target->*func)()) {
                return true;
            }
        }

        return false;
    }

    QList<SWT_Checker> targetsFunctionList;
};

#endif // SINGLEWIDGETTARGET_H
