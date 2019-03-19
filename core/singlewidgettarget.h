#ifndef SINGLEWIDGETTARGET_H
#define SINGLEWIDGETTARGET_H
#include <QList>
#include "singlewidgetabstraction.h"

class QMenu;
class QAction;
class QMimeData;

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

typedef bool (SingleWidgetTarget::*SWT_Checker)() const;

struct SWT_RulesCollection {
    SWT_RulesCollection();
    SWT_RulesCollection(const SWT_Rule &ruleT,
                        const bool &alwaysShowChildrenT,
                        const SWT_Target &targetT,
                        SWT_Checker typeT,
                        const QString &searchStringT) {
        rule = ruleT;
        alwaysShowChildren = alwaysShowChildrenT;
        fTarget = targetT;
        type = typeT;
        fSearchString = searchStringT;
    }

    SWT_Rule rule;
    bool alwaysShowChildren;
    SWT_Target fTarget;
    SWT_Checker type;
    QString fSearchString;
};

enum SWT_Target : short {
    SWT_CurrentCanvas,
    SWT_CurrentGroup,
    SWT_All
};


class SingleWidgetTarget : public SelfRef {
public:
    SingleWidgetTarget() {}
    virtual void SWT_addChildrenAbstractions(
            SingleWidgetAbstraction*,
            const UpdateFuncs &,
            const int&) {}

    virtual bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                                     const bool &parentSatisfies,
                                     const bool &parentMainTarget) const {
        Q_UNUSED(rules);
        return parentSatisfies && !parentMainTarget;
    }

    virtual QMimeData *SWT_createMimeData() {
        return nullptr;
    }

    virtual void SWT_setChildrenAncestorDisabled(const bool &bT) {
        Q_UNUSED(bT);
    }

    virtual bool SWT_visibleOnlyIfParentDescendant() const {
        return true;
    }

    // Animators
    virtual bool SWT_isAnimator() const { return false; }
    virtual bool SWT_isGraphAnimator() const { return false; }
    virtual bool SWT_isBoolAnimator() const { return false; }
    virtual bool SWT_isColorAnimator() const { return false; }
    virtual bool SWT_isComplexAnimator() const { return false; }
    virtual bool SWT_isFakeComplexAnimator() const { return false; }
    virtual bool SWT_isPixmapEffectAnimators() const { return false; }
    virtual bool SWT_isPathEffectAnimators() const { return false; }
    virtual bool SWT_isPixmapEffect() const { return false; }
    virtual bool SWT_isPathEffect() const { return false; }
    virtual bool SWT_isIntAnimator() const { return false; }
    virtual bool SWT_isGradient() const { return false; }
    virtual bool SWT_isFillSettingsAnimator() const { return false; }
    virtual bool SWT_isStrokeSettings() const { return false; }
    virtual bool SWT_isPathAnimator() const { return false; }
    virtual bool SWT_isQPointFAnimator() const { return false; }
    virtual bool SWT_isQrealAnimator() const { return false; }
    virtual bool SWT_isQStringAnimator() const { return false; }
    virtual bool SWT_isBasicTransformAnimator() const { return false; }
    virtual bool SWT_isBoxTransformAnimator() const { return false; }
    virtual bool SWT_isVectorPathAnimator() const { return false; }
    virtual bool SWT_isSmartPathAnimator() const { return false; }
    virtual bool SWT_isAnimatedSurface() const { return false; }
    virtual bool SWT_isQCubicSegment1DAnimator() const { return false; }
    // Boxes
    virtual bool SWT_isCanvas() const { return false; }
    virtual bool SWT_isAnimationBox() const { return false; }
    virtual bool SWT_isBoundingBox() const { return false; }
    virtual bool SWT_isBonesBox() const { return false; }
    virtual bool SWT_isBone() const { return false; }
    virtual bool SWT_isBoxesGroup() const { return false; }
    virtual bool SWT_isCircle() const { return false; }
    virtual bool SWT_isImageBox() const { return false; }
    virtual bool SWT_isImageSequenceBox() const { return false; }
    virtual bool SWT_isLinkBox() const { return false; }
    virtual bool SWT_isParticleBox() const { return false; }
    virtual bool SWT_isPathBox() const { return false; }
    virtual bool SWT_isRectangle() const { return false; }
    virtual bool SWT_isTextBox() const { return false; }
    virtual bool SWT_isVectorPath() const { return false; }
    virtual bool SWT_isVideoBox() const { return false; }
    virtual bool SWT_isPaintBox() const { return false; }
    // Properties
    virtual bool SWT_isBoolProperty() const { return false; }
    virtual bool SWT_isBoolPropertyContainer() const { return false; }
    virtual bool SWT_isComboBoxProperty() const { return false; }
    virtual bool SWT_isBoxTargetProperty() const { return false; }
    virtual bool SWT_isProperty() const { return false; }
    virtual bool SWT_isIntProperty() const { return false; }
    // Sound
    virtual bool SWT_isSingleSound() const { return false; }

    void SWT_addChildAbstractionForTargetToAll(
            SingleWidgetTarget * const target);
    void SWT_addChildAbstractionForTargetToAllAt(
            SingleWidgetTarget * const target, const int &id);
    void SWT_removeChildAbstractionForTargetFromAll(
            SingleWidgetTarget * const target);



    SingleWidgetAbstraction *SWT_createAbstraction(
            const UpdateFuncs &updateFuncs,
            const int &visiblePartWidgetId);
    void SWT_removeAbstractionForWidget(const int& visiblePartWidgetId) {
        const auto currSwa = SWT_getAbstractionForWidget(visiblePartWidgetId);
        if(!currSwa) return;
        const auto currSwaSPtr = GetAsSPtr(currSwa, SingleWidgetAbstraction);
        SWT_removeAbstractionFromList(currSwaSPtr);
    }

    void SWT_removeAbstractionFromList(
            const stdsptr<SingleWidgetAbstraction> &abs);

    SingleWidgetAbstraction* SWT_getAbstractionForWidget(
            const int& visiblePartWidgetId) const {
        for(const auto& abs : SWT_mAllAbstractions) {
            if(abs->getParentVisiblePartWidgetId() == visiblePartWidgetId) {
                return abs.get();
            }
        }
        return nullptr;
    }

    SingleWidgetAbstraction* SWT_getOrCreateAbstractionForWidget(
            const UpdateFuncs &updateFuncs,
            const int& visiblePartWidgetId) {
        const auto curr = SWT_getAbstractionForWidget(visiblePartWidgetId);
        if(curr) return curr;
        return SWT_createAbstraction(updateFuncs, visiblePartWidgetId);
    }

    void SWT_scheduleWidgetsContentUpdateWithRule(
            const SWT_Rule &rule);

    void SWT_scheduleWidgetsContentUpdateWithSearchNotEmpty();
    void SWT_scheduleWidgetsContentUpdateWithTarget(
            SingleWidgetTarget *targetP, const SWT_Target &target);

    void SWT_moveChildAbstractionForTargetToInAll(
            SingleWidgetTarget * const target, const int &id);

    bool SWT_isVisible() const {
        return SWT_mVisible;
    }

    void SWT_hide() {
        SWT_setVisible(false);
    }

    void SWT_show() {
        SWT_setVisible(true);
    }

    void SWT_setVisible(const bool &bT) {
        SWT_mVisible = bT;
        SWT_afterContentVisibilityChanged();
    }

    void SWT_setDisabled(const bool &disable) {
        SWT_mDisabled = disable;
        SWT_setChildrenAncestorDisabled(SWT_isDisabled());
    }

    void SWT_disable() {
        SWT_setDisabled(true);
    }

    void SWT_enable() {
        SWT_setDisabled(false);
    }

    bool SWT_isDisabled() const {
        return SWT_mDisabled || SWT_mAncestorDisabled;
    }

    void SWT_setAncestorDisabled(const bool &bT) {
        SWT_mAncestorDisabled = bT;
        SWT_setChildrenAncestorDisabled(SWT_isDisabled());
    }

    void SWT_afterContentVisibilityChanged();
protected:
    bool SWT_mAncestorDisabled = false;
    bool SWT_mVisible = true;
    bool SWT_mDisabled = false;
    QList<stdsptr<SingleWidgetAbstraction>> SWT_mAllAbstractions;
};

struct SWT_TargetTypes {
    bool isTargeted(SingleWidgetTarget * const target) const {
        if(!target) return false;
        for(SWT_Checker func : fTargetsFunctionList) {
            if((target->*func)()) return true;
        }
        return false;
    }

    QList<SWT_Checker> fTargetsFunctionList;
};

#endif // SINGLEWIDGETTARGET_H
