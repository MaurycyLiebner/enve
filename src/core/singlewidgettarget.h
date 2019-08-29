// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef SINGLEWIDGETTARGET_H
#define SINGLEWIDGETTARGET_H
#include <QList>
#include <map>
#include "singlewidgetabstraction.h"

class QMenu;
class QAction;
class QMimeData;

enum SWT_BoxRule : short {
    SWT_BR_ALL,
    SWT_BR_SELECTED,
    SWT_BR_VISIBLE,
    SWT_BR_HIDDEN,
    SWT_BR_LOCKED,
    SWT_BR_UNLOCKED,
    SWT_BR_ANIMATED,
    SWT_BR_NOT_ANIMATED
};


enum SWT_Target : short {
    SWT_TARGET_CURRENT_CANVAS,
    SWT_TARGET_CURRENT_GROUP,
    SWT_TARGET_ALL
};

enum SWT_Type : short {
    SWT_TYPE_ALL,
    SWT_TYPE_GRAPHICS,
    SWT_TYPE_SOUND
};

struct SWT_RulesCollection {
    SWT_RulesCollection();
    SWT_RulesCollection(const SWT_BoxRule &ruleT,
                        const bool alwaysShowChildrenT,
                        const SWT_Target targetT,
                        const SWT_Type& typeT,
                        const QString &searchStringT) {
        fRule = ruleT;
        fAlwaysShowChildren = alwaysShowChildrenT;
        fTarget = targetT;
        fType = typeT;
        fSearchString = searchStringT;
    }

    SWT_BoxRule fRule;
    bool fAlwaysShowChildren;
    SWT_Target fTarget;
    SWT_Type fType;
    QString fSearchString;
};

class SingleWidgetTarget : public SelfRef {
    Q_OBJECT
public:
    SingleWidgetTarget() {}
    virtual void SWT_setupAbstraction(
            SWT_Abstraction*,
            const UpdateFuncs &,
            const int) {}

    virtual bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                                     const bool parentSatisfies,
                                     const bool parentMainTarget) const {
        Q_UNUSED(rules);
        return parentSatisfies && !parentMainTarget;
    }

    virtual QMimeData *SWT_createMimeData() {
        return nullptr;
    }

    virtual bool SWT_dropSupport(const QMimeData* const data) {
        Q_UNUSED(data);
        return false;
    }

    virtual bool SWT_drop(const QMimeData* const data) {
        Q_UNUSED(data);
        return false;
    }

    virtual bool SWT_dropIntoSupport(const int index,
                                     const QMimeData* const data) {
        Q_UNUSED(index);
        Q_UNUSED(data);
        return false;
    }

    virtual bool SWT_dropInto(const int index,
                              const QMimeData* const data) {
        Q_UNUSED(index);
        Q_UNUSED(data);
        return false;
    }

    virtual void SWT_setChildrenAncestorDisabled(const bool bT) {
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
    virtual bool SWT_isRasterEffectAnimators() const { return false; }
    virtual bool SWT_isPathEffectAnimators() const { return false; }
    virtual bool SWT_isRasterEffect() const { return false; }
    virtual bool SWT_isPathEffect() const { return false; }
    virtual bool SWT_isIntAnimator() const { return false; }
    virtual bool SWT_isGradient() const { return false; }
    virtual bool SWT_isFillSettingsAnimator() const { return false; }
    virtual bool SWT_isStrokeSettings() const { return false; }
    virtual bool SWT_isSmartPathCollection() const { return false; }
    virtual bool SWT_isQPointFAnimator() const { return false; }
    virtual bool SWT_isQrealAnimator() const { return false; }
    virtual bool SWT_isQStringAnimator() const { return false; }
    virtual bool SWT_isBasicTransformAnimator() const { return false; }
    virtual bool SWT_isBoxTransformAnimator() const { return false; }
    virtual bool SWT_isSmartPathAnimator() const { return false; }
    virtual bool SWT_isAnimatedSurface() const { return false; }
    virtual bool SWT_isQCubicSegment1DAnimator() const { return false; }
    // Boxes
    virtual bool SWT_isCanvas() const { return false; }
    virtual bool SWT_isAnimationBox() const { return false; }
    virtual bool SWT_isBoundingBox() const { return false; }
    virtual bool SWT_isLayerBox() const { return false; }
    virtual bool SWT_isGroupBox() const { return false; }
    virtual bool SWT_isContainerBox() const { return false; }
    virtual bool SWT_isCircle() const { return false; }
    virtual bool SWT_isImageBox() const { return false; }
    virtual bool SWT_isImageSequenceBox() const { return false; }
    virtual bool SWT_isLinkBox() const { return false; }
    virtual bool SWT_isPathBox() const { return false; }
    virtual bool SWT_isRectangle() const { return false; }
    virtual bool SWT_isTextBox() const { return false; }
    virtual bool SWT_isVideoBox() const { return false; }
    virtual bool SWT_isPaintBox() const { return false; }
    virtual bool SWT_isSmartVectorPath() const { return false; }
    // Properties
    virtual bool SWT_isBoolProperty() const { return false; }
    virtual bool SWT_isBoolPropertyContainer() const { return false; }
    virtual bool SWT_isComboBoxProperty() const { return false; }
    virtual bool SWT_isBoxTargetProperty() const { return false; }
    virtual bool SWT_isProperty() const { return false; }
    virtual bool SWT_isIntProperty() const { return false; }
    // Sound
    virtual bool SWT_isSingleSound() const { return false; }
    virtual bool SWT_isSound() const { return false; }

    void SWT_addChild(SingleWidgetTarget * const child);
    void SWT_addChildAt(SingleWidgetTarget * const child, const int id);
    void SWT_removeChild(SingleWidgetTarget * const child);

    SWT_Abstraction *SWT_createAbstraction(
            const UpdateFuncs &updateFuncs,
            const int visiblePartWidgetId);

    void SWT_removeAbstractionForWidget(const int visiblePartWidgetId) {
        SWT_mAllAbstractions.erase(visiblePartWidgetId);
    }

    SWT_Abstraction* SWT_getAbstractionForWidget(
            const int visiblePartWidgetId) const {
        const auto it = SWT_mAllAbstractions.find(visiblePartWidgetId);
        if(it == SWT_mAllAbstractions.end()) return nullptr;
        return it->second.get();
    }

    SWT_Abstraction* SWT_abstractionForWidget(
            const UpdateFuncs &updateFuncs,
            const int visiblePartWidgetId) {
        const auto curr = SWT_getAbstractionForWidget(visiblePartWidgetId);
        if(curr) return curr;
        return SWT_createAbstraction(updateFuncs, visiblePartWidgetId);
    }

    void SWT_scheduleContentUpdate(const SWT_BoxRule rule);

    void SWT_scheduleSearchContentUpdate();

    void SWT_moveChildTo(SingleWidgetTarget * const target, const int id);

    bool SWT_isVisible() const {
        return SWT_mVisible;
    }

    void SWT_hide() {
        SWT_setVisible(false);
    }

    void SWT_show() {
        SWT_setVisible(true);
    }

    void SWT_setVisible(const bool bT) {
        if(SWT_mVisible == bT) return;
        SWT_mVisible = bT;
        for(const auto &swa : SWT_mAllAbstractions) {
            swa.second->afterContentVisibilityChanged();
        }
    }

    void SWT_setEnabled(const bool enabled) {
        SWT_setDisabled(!enabled);
    }

    void SWT_setDisabled(const bool disable) {
        if(SWT_mDisabled == disable) return;
        SWT_mDisabled = disable;
        SWT_setChildrenAncestorDisabled(SWT_isDisabled());
        emit SWT_changedDisabled(SWT_isDisabled());
    }

    void SWT_disable() {
        SWT_setDisabled(true);
    }

    void SWT_enable() {
        SWT_setDisabled(false);
    }

    bool SWT_isEnabled() const {
        return !SWT_isDisabled();
    }

    bool SWT_isDisabled() const {
        return SWT_mDisabled || SWT_mAncestorDisabled;
    }

    void SWT_setAncestorDisabled(const bool disabled) {
        if(SWT_mAncestorDisabled == disabled) return;
        SWT_mAncestorDisabled = disabled;
        SWT_setChildrenAncestorDisabled(SWT_isDisabled());
        emit SWT_changedDisabled(SWT_isDisabled());
    }
signals:
    void SWT_changedDisabled(bool);
private:
    bool SWT_mAncestorDisabled = false;
    bool SWT_mVisible = true;
    bool SWT_mDisabled = false;
    std::map<int, stdsptr<SWT_Abstraction>> SWT_mAllAbstractions;
};

#endif // SINGLEWIDGETTARGET_H
