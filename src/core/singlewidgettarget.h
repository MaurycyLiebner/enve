// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

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
#include "swt_abstraction.h"
#include "swt_rulescollection.h"

class QMenu;
class QAction;
class QMimeData;

class SingleWidgetTarget : public SelfRef {
    Q_OBJECT
public:
    SingleWidgetTarget() {}
    virtual void SWT_setupAbstraction(SWT_Abstraction*,
                                      const UpdateFuncs &,
                                      const int) {}

    virtual bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                                     const bool parentSatisfies,
                                     const bool parentMainTarget) const {
        Q_UNUSED(rules)
        return parentSatisfies && !parentMainTarget;
    }

    virtual QMimeData *SWT_createMimeData() {
        return nullptr;
    }

    virtual bool SWT_dropSupport(const QMimeData* const data) {
        Q_UNUSED(data)
        return false;
    }

    virtual bool SWT_drop(const QMimeData* const data) {
        Q_UNUSED(data)
        return false;
    }

    virtual bool SWT_dropIntoSupport(const int index,
                                     const QMimeData* const data) {
        Q_UNUSED(index)
        Q_UNUSED(data)
        return false;
    }

    virtual bool SWT_dropInto(const int index,
                              const QMimeData* const data) {
        Q_UNUSED(index)
        Q_UNUSED(data)
        return false;
    }

    virtual void SWT_setChildrenAncestorDisabled(const bool bT) {
        Q_UNUSED(bT)
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
    virtual bool SWT_isPathEffectCollection() const { return false; }
    virtual bool SWT_isPathEffect() const { return false; }
    virtual bool SWT_isIntAnimator() const { return false; }
    virtual bool SWT_isGradient() const { return false; }
    virtual bool SWT_isFillSettingsAnimator() const { return false; }
    virtual bool SWT_isStrokeSettings() const { return false; }
    virtual bool SWT_isSmartPathCollection() const { return false; }
    virtual bool SWT_isSculptPathCollection() const { return false; }
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
    virtual bool SWT_isSculptPathBox() const { return false; }
    // Properties
    virtual bool SWT_isBoolProperty() const { return false; }
    virtual bool SWT_isBoolPropertyContainer() const { return false; }
    virtual bool SWT_isComboBoxProperty() const { return false; }
    virtual bool SWT_isBoxTargetProperty() const { return false; }
    virtual bool SWT_isProperty() const { return false; }
    // Sound
    virtual bool SWT_isSingleSound() const { return false; }
    virtual bool SWT_isSound() const { return false; }

    void SWT_addChild(SingleWidgetTarget * const child);
    void SWT_addChildAt(SingleWidgetTarget * const child, const int id);
    void SWT_removeChild(SingleWidgetTarget * const child);

    SWT_Abstraction *SWT_createAbstraction(const UpdateFuncs &updateFuncs,
                                           const int visiblePartWidgetId);

    void SWT_removeAbstractionForWidget(const int visiblePartWidgetId);

    SWT_Abstraction* SWT_getAbstractionForWidget(
            const int visiblePartWidgetId) const;

    SWT_Abstraction* SWT_abstractionForWidget(
            const UpdateFuncs &updateFuncs,
            const int visiblePartWidgetId);

    void SWT_scheduleContentUpdate(const SWT_BoxRule rule);

    void SWT_scheduleSearchContentUpdate();

    void SWT_moveChildTo(SingleWidgetTarget * const target, const int id);

    bool SWT_isVisible() const { return SWT_mVisible; }

    void SWT_hide() { SWT_setVisible(false); }
    void SWT_show() { SWT_setVisible(true); }
    void SWT_setVisible(const bool bT);

    void SWT_setEnabled(const bool enabled)
    { SWT_setDisabled(!enabled); }

    void SWT_setDisabled(const bool disable);

    void SWT_disable()
    { SWT_setDisabled(true); }

    void SWT_enable()
    { SWT_setDisabled(false); }

    bool SWT_isEnabled() const
    { return !SWT_isDisabled(); }

    bool SWT_isDisabled() const
    { return SWT_mDisabled || SWT_mAncestorDisabled; }

    void SWT_setAncestorDisabled(const bool disabled);
signals:
    void SWT_changedDisabled(bool);
private:
    bool SWT_mAncestorDisabled = false;
    bool SWT_mVisible = true;
    bool SWT_mDisabled = false;
    std::map<int, stdsptr<SWT_Abstraction>> SWT_mAllAbstractions;
};

#endif // SINGLEWIDGETTARGET_H
