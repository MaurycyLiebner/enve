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

#ifndef CONTAINERBOX_H
#define CONTAINERBOX_H
#include "boxwithpatheffects.h"
#include "conncontextobjlist.h"
class PathBox;
class PathEffectCollection;

class ContainerBox : public BoxWithPathEffects {
    Q_OBJECT
    e_OBJECT
    typedef qCubicSegment1DAnimator::Action SegAction;
protected:
    ContainerBox(const eBoxType type);
public:
    bool SWT_isContainerBox() const { return true; }
    bool SWT_isGroupBox() const { return mType == eBoxType::group; }
    bool SWT_isLayerBox() const { return !SWT_isGroupBox(); }

    HardwareSupport hardwareSupport() const {
        if(SWT_isLayerBox()) return HardwareSupport::gpuPreffered;
        return BoundingBox::hardwareSupport();
    }

    bool SWT_dropSupport(const QMimeData* const data);
    bool SWT_dropIntoSupport(const int index, const QMimeData* const data);
    bool SWT_drop(const QMimeData* const data);
    bool SWT_dropInto(const int index, const QMimeData* const data);


    void drawContained(SkCanvas * const canvas,
                       const SkFilterQuality filter);
    void drawContained(SkCanvas * const canvas,
                       const SkFilterQuality filter, int& drawId,
                       QList<BlendEffect::Delayed>& delayed);
    void drawPixmapSk(SkCanvas * const canvas,
                      const SkFilterQuality filter, int &drawId,
                      QList<BlendEffect::Delayed> &delayed);

    qsptr<BoundingBox> createLink(const bool inner);
    stdsptr<BoxRenderData> createRenderData();
    void setupRenderData(const qreal relFrame,
                         BoxRenderData * const data,
                         Canvas * const scene);

    virtual BoundingBox *getBoxAt(const QPointF &absPos);

    void anim_setAbsFrame(const int frame);

    Property* ca_findPropertyWithPath(
            const int id, const QStringList& path,
            QStringList * const completions) const;

    BoundingBox *getBoxAtFromAllDescendents(const QPointF &absPos);
    void anim_scaleTime(const int pivotAbsFrame, const qreal scale);
    void updateAllBoxes(const UpdateReason reason);

    bool relPointInsidePath(const QPointF &relPos) const;
    void SWT_setupAbstraction(SWT_Abstraction *abstraction,
                                     const UpdateFuncs &updateFuncs,
                                     const int visiblePartWidgetId);
    bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                             const bool parentSatisfies,
                             const bool parentMainTarget) const;
    void setupCanvasMenu(PropertyMenu * const menu);

    FrameRange prp_getIdenticalRelRange(const int relFrame) const;
    FrameRange getFirstAndLastIdenticalForMotionBlur(
            const int relFrame, const bool takeAncestorsIntoAccount = true);

    void prp_afterFrameShiftChanged(const FrameRange& oldAbsRange,
                                    const FrameRange& newAbsRange);
    void shiftAll(const int shift);

    void strokeWidthAction(const QrealAction& action);

    void startSelectedStrokeColorTransform();
    void startSelectedFillColorTransform();
    void applyPaintSetting(const PaintSettingsApplier &setting);

    void setStrokeCapStyle(const SkPaint::Cap capStyle);
    void setStrokeJoinStyle(const SkPaint::Join joinStyle);

    void setStrokeBrush(SimpleBrushWrapper * const brush);

    void applyStrokeBrushWidthAction(const SegAction& action);
    void applyStrokeBrushPressureAction(const SegAction& action);
    void applyStrokeBrushSpacingAction(const SegAction& action);
    void applyStrokeBrushTimeAction(const SegAction& action);

    FillSettingsAnimator *getFillSettings() const;
    OutlineSettingsAnimator *getStrokeSettings() const;

    bool shouldScheduleUpdate() { return !SWT_isGroupBox(); }

    void queChildrenTasks();
    void queTasks();

    void writeAllContained(eWriteStream &dst) const;
    void writeBoundingBox(eWriteStream& dst) const;
    void readContained(eReadStream &src);
    void readAllContained(eReadStream &src);
    void readBoundingBox(eReadStream& src);

    void promoteToLayer();
    void demoteToGroup();

    const QList<BoundingBox*> &getContainedBoxes() const;
    const auto &getContained() const { return mContained; }

    void forcedMarginMeaningfulChange();
    QRect currentGlobalBounds() const;

    bool diffsAffectingContainedBoxes(const int relFrame1,
                                      const int relFrame2);

    void deselectAllBoxesFromBoxesGroup();
    void selectAllBoxesFromBoxesGroup();
    void addContainedBoxesToSelection(const QRectF &rect);

    void setIsCurrentGroup_k(const bool bT);

    void ungroupAction_k();
    void ungroupKeepTransform_k();
    void ungroupAbandomTransform_k();

    bool isCurrentGroup() const;

    void updateContainedBoxes();
    bool replaceContained(const qsptr<eBoxOrSound>& replaced,
                             const qsptr<eBoxOrSound>& replacer);
    void addContained(const qsptr<eBoxOrSound> &child);
    void insertContained(const int id, const qsptr<eBoxOrSound> &child);
    void updateContainedIds(const int firstId);
    void updateContainedIds(const int firstId, const int lastId);
    int getContainedIndex(eBoxOrSound * const child);
    qsptr<eBoxOrSound> takeContained_k(const int id);
    void removeContained_k(const qsptr<eBoxOrSound> &child);
    void increaseContainedZInList(eBoxOrSound * const child);
    void decreaseContainedZInList(eBoxOrSound * const child);
    void bringContainedToEndList(eBoxOrSound * const child);
    void bringContainedToFrontList(eBoxOrSound * const child);
    void moveContainedInList(eBoxOrSound * const child, const int to);
    void moveContainedInList(const int from, const int to);
    void moveContainedInList(eBoxOrSound * const child,
                                const int from, const int to);
    void moveContainedBelow(eBoxOrSound * const boxToMove,
                            eBoxOrSound * const below);
    void moveContainedAbove(eBoxOrSound * const boxToMove,
                            eBoxOrSound * const above);

    void removeContainedFromList(const int id);
    void setDescendantCurrentGroup(const bool bT);
    bool isDescendantCurrentGroup() const;

    int abstractionIdToBoxId(const int absId) const {
        return absId - ca_getNumberOfChildren();
    }

    int containedIdToAbstractionId(const int contId) const {
        return contId + ca_getNumberOfChildren();
    }

    bool areAllChildrenStatic();
    int getContainedBoxesCount() const;
    void removeAllContained();

    QString makeNameUniqueForDescendants(
            const QString& name, eBoxOrSound * const skip = nullptr);
    QString makeNameUniqueForContained(
            const QString& name, eBoxOrSound * const skip = nullptr);

    QStringList allDescendantsNamesStartingWith(
            const QString& text, eBoxOrSound * const skip = nullptr);
    QStringList allContainedNamesStartingWith(
            const QString& text, eBoxOrSound * const skip = nullptr);

    void allDescendantsStartingWith(
            const QString& text, QList<eBoxOrSound*> &result);
    void allContainedStartingWith(
            const QString& text, QList<eBoxOrSound*> &result);

    void addBoxWithBlendEffects(BoundingBox* const box)
    { mBoxesWithBlendEffects << box; }
    void removeBoxWithBlendEffects(BoundingBox* const box)
    { mBoxesWithBlendEffects.removeOne(box); }

    void addAllChildBoxesWithBlendEffects(ContainerBox * const layer);
    void removeAllChildBoxesWithBlendEffects(ContainerBox* const layer);

    void containedDetachedBlendSetup(
            SkCanvas * const canvas,
            const SkFilterQuality filter, int& drawId,
            QList<BlendEffect::Delayed> &delayed) const;

    void updateIfUsesProgram(const ShaderEffectProgram * const program) const final;
signals:
    void switchedGroupLayer();
private:
    void updateAllChildPaths(const UpdateReason reason,
                             void (PathBox::*func)(const UpdateReason));

    void iniPathEffects();
    void updateRelBoundingRect();
protected:
    void removeContained(const qsptr<eBoxOrSound> &child);

    QMargins mForcedMargin;
    
    bool mIsCurrentGroup = false;
    bool mIsDescendantCurrentGroup = false;
    QList<BoundingBox*> mBoxesWithBlendEffects;
    QList<BoundingBox*> mContainedBoxes;
    ConnContextObjList<qsptr<eBoxOrSound>> mContained;
};

#endif // CONTAINERBOX_H
