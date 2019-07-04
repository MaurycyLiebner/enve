#ifndef CONTAINERBOX_H
#define CONTAINERBOX_H
#include "boundingbox.h"
class PathBox;
class PathEffectAnimators;

class ContainerBox : public BoundingBox {
    friend class SelfRef;
protected:
    ContainerBox(const BoundingBoxType &type);
public:
    bool SWT_isContainerBox() const { return true; }
    bool SWT_isGroupBox() const { return mType == TYPE_GROUP; }
    bool SWT_isLayerBox() const { return !SWT_isGroupBox(); }

    void drawPixmapSk(SkCanvas * const canvas,
                      GrContext * const grContext);

    qsptr<BoundingBox> createLink();
    stdsptr<BoundingBoxRenderData> createRenderData();
    void setupRenderData(const qreal relFrame,
                         BoundingBoxRenderData * const data);

    virtual BoundingBox *getBoxAt(const QPointF &absPos);

    void anim_setAbsFrame(const int frame);

    BoundingBox *getBoxAtFromAllDescendents(const QPointF &absPos);
    void anim_scaleTime(const int pivotAbsFrame, const qreal scale);
    void updateAllBoxes(const UpdateReason &reason);

    QRectF getRelBoundingRect(const qreal relFrame);

    bool relPointInsidePath(const QPointF &relPos) const;
    void SWT_setupAbstraction(SWT_Abstraction *abstraction,
                                     const UpdateFuncs &updateFuncs,
                                     const int visiblePartWidgetId);
    bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                             const bool parentSatisfies,
                             const bool parentMainTarget) const;
    void addActionsToMenu(BoxTypeMenu * const menu);

    FrameRange prp_getIdenticalRelRange(const int relFrame) const;
    FrameRange getFirstAndLastIdenticalForMotionBlur(
            const int relFrame, const bool takeAncestorsIntoAccount = true);

    void prp_afterFrameShiftChanged();
    void shiftAll(const int shift);

    void addPathEffect(const qsptr<PathEffect>& effect);
    void addFillPathEffect(const qsptr<PathEffect>& effect);
    void addOutlineBasePathEffect(const qsptr<PathEffect>& effect);
    void addOutlinePathEffect(const qsptr<PathEffect>& effect);
    void removePathEffect(const qsptr<PathEffect>& effect);
    void removeFillPathEffect(const qsptr<PathEffect>& effect);
    void removeOutlinePathEffect(const qsptr<PathEffect>& effect);

    void startSelectedStrokeWidthTransform();
    void startSelectedStrokeColorTransform();
    void startSelectedFillColorTransform();
    void applyPaintSetting(const PaintSettingsApplier &setting);
    void setFillColorMode(const ColorMode colorMode);
    void setStrokeColorMode(const ColorMode colorMode);

    void setStrokeCapStyle(const SkPaint::Cap capStyle);
    void setStrokeJoinStyle(const SkPaint::Join joinStyle);
    void setStrokeWidth(const qreal strokeWidth);
    void setStrokeBrush(SimpleBrushWrapper * const brush) {
        for(const auto& box : mContainedBoxes) {
            box->setStrokeBrush(brush);
        }
    }
    void setStrokeBrushWidthCurve(const qCubicSegment1D& curve) {
        for(const auto& box : mContainedBoxes)
            box->setStrokeBrushWidthCurve(curve);
    }
    void setStrokeBrushTimeCurve(const qCubicSegment1D& curve) {
        for(const auto& box : mContainedBoxes)
            box->setStrokeBrushTimeCurve(curve);
    }
    void setStrokeBrushPressureCurve(const qCubicSegment1D& curve) {
        for(const auto& box : mContainedBoxes)
            box->setStrokeBrushPressureCurve(curve);
    }
    void setStrokeBrushSpacingCurve(const qCubicSegment1D& curve) {
        for(const auto& box : mContainedBoxes)
            box->setStrokeBrushSpacingCurve(curve);
    }
    FillSettingsAnimator *getFillSettings() const;
    OutlineSettingsAnimator *getStrokeSettings() const;

    void scheduleWaitingTasks();
    void queScheduledTasks();


    void writeBoundingBox(QIODevice * const target);
    void readBoundingBox(QIODevice * const target);

    void promoteToLayer();
    void demoteToGroup();

    const QList<qsptr<BoundingBox>> &getContainedBoxesList() const;

    //    bool anim_nextRelFrameWithKey(const int relFrame,
    //                                 int &nextRelFrame);
    //    bool anim_prevRelFrameWithKey(const int relFrame,
    //                                 int &prevRelFrame);

    bool differenceInPathEffectsBetweenFrames(const int relFrame1,
                                              const int relFrame2) const;
    bool differenceInFillPathEffectsBetweenFrames(const int relFrame1,
                                                  const int relFrame2) const;
    bool differenceInOutlinePathEffectsBetweenFrames(const int relFrame1,
                                                     const int relFrame2) const;
    void updateAllChildPathBoxes(const UpdateReason &reason);

    void applyPathEffects(const qreal relFrame,
                          SkPath * const srcDstPath,
                          BoundingBox * const box);

    void filterOutlineBasePath(const qreal relFrame,
                                          SkPath * const srcDstPath);
    void filterOutlinePath(const qreal relFrame,
                           SkPath * const srcDstPath);
    void filterFillPath(const qreal relFrame,
                        SkPath * const srcDstPath);

    bool diffsAffectingContainedBoxes(const int relFrame1,
                                      const int relFrame2);

    void deselectAllBoxesFromBoxesGroup();
    void selectAllBoxesFromBoxesGroup();
    void addContainedBoxesToSelection(const QRectF &rect);

    void setIsCurrentGroup_k(const bool bT);
    void ungroup_k();

    bool isCurrentGroup() const;
    bool replaceContainedBox(const qsptr<BoundingBox>& replaced,
                             const qsptr<BoundingBox>& replacer);
    void addContainedBox(const qsptr<BoundingBox> &child);
    void addContainedBoxToListAt(const int index,
                                 const qsptr<BoundingBox> &child);
    void updateContainedBoxIds(const int firstId);
    void updateContainedBoxIds(const int firstId,
                              const int lastId);
    int getContainedBoxIndex(BoundingBox * const child);
    qsptr<BoundingBox> takeContainedBox_k(const int id);
    void removeContainedBox_k(const qsptr<BoundingBox> &child);
    void increaseContainedBoxZInList(BoundingBox * const child);
    void decreaseContainedBoxZInList(BoundingBox * const child);
    void bringContainedBoxToEndList(BoundingBox * const child);
    void bringContainedBoxToFrontList(BoundingBox * const child);
    void moveContainedBoxInList(BoundingBox * const child, const int to);
    void moveContainedBoxInList(BoundingBox * const child,
                                const int from,
                                const int to);
    void moveContainedBoxBelow(BoundingBox * const boxToMove,
                               BoundingBox * const below);
    void moveContainedBoxAbove(BoundingBox * const boxToMove,
                               BoundingBox * const above);

    void removeContainedBoxFromList(const int id);
    void setDescendantCurrentGroup(const bool bT);
    bool isDescendantCurrentGroup() const;

    void readChildBoxes(QIODevice *target);

    int abstractionIdToBoxId(const int absId) const {
        if(absId < ca_getNumberOfChildren()) return -1;
        const int revId = absId - ca_getNumberOfChildren();
        return mContainedBoxes.count() - revId - 1;
    }

    int boxIdToAbstractionId(const int boxId) const {
        const int revId = mContainedBoxes.count() - boxId - 1;
        return revId + ca_getNumberOfChildren();
    }
    int getContainedBoxesCount() const;
    void removeAllContainedBoxes();

    void updateIfUsesProgram(
            const GPURasterEffectProgram * const program) const final;
protected:
    void removeContainedBox(const qsptr<BoundingBox> &child);

    bool mIsCurrentGroup = false;
    bool mIsDescendantCurrentGroup = false;
    QList<qsptr<BoundingBox>> mContainedBoxes;

    qsptr<PathEffectAnimators> mPathEffectsAnimators;
    qsptr<PathEffectAnimators> mFillPathEffectsAnimators;
    qsptr<PathEffectAnimators> mOutlineBasePathEffectsAnimators;
    qsptr<PathEffectAnimators> mOutlinePathEffectsAnimators;
private:
    void setupLayerRenderData(const qreal relFrame,
                              BoundingBoxRenderData * const data);
    void iniPathEffects();
};

#endif // CONTAINERBOX_H
