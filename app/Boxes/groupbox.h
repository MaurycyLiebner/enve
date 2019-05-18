#ifndef GROUPBOX_H
#define GROUPBOX_H
#include "boundingbox.h"
class PathBox;
class PathEffectAnimators;

class GroupBox : public BoundingBox {
    Q_OBJECT
    friend class SelfRef;
protected:
    GroupBox(const BoundingBoxType &type = TYPE_GROUP);
public:
    virtual BoundingBox *getBoxAt(const QPointF &absPos);

    bool SWT_isGroupBox() const { return true; }
    BoundingBox *getBoxAtFromAllDescendents(const QPointF &absPos);
    void anim_scaleTime(const int& pivotAbsFrame, const qreal& scale);
    void updateAllBoxes(const UpdateReason &reason);

    QRectF getRelBoundingRect(const qreal &relFrame);

    bool relPointInsidePath(const QPointF &relPos) const;
    void SWT_addChildrenAbstractions(SingleWidgetAbstraction *abstraction,
                                     const UpdateFuncs &updateFuncs,
                                     const int& visiblePartWidgetId);
    bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                             const bool &parentSatisfies,
                             const bool &parentMainTarget) const;
    void addActionsToMenu(BoxTypeMenu * const menu);

    FrameRange prp_getIdenticalRelRange(const int &relFrame) const;
    FrameRange getFirstAndLastIdenticalForMotionBlur(
            const int &relFrame, const bool &takeAncestorsIntoAccount = true);

    void prp_afterFrameShiftChanged();
    void shiftAll(const int &shift);

    void addPathEffect(const qsptr<PathEffect>& effect);
    void addFillPathEffect(const qsptr<PathEffect>& effect);
    void addOutlinePathEffect(const qsptr<PathEffect>& effect);
    void removePathEffect(const qsptr<PathEffect>& effect);
    void removeFillPathEffect(const qsptr<PathEffect>& effect);
    void removeOutlinePathEffect(const qsptr<PathEffect>& effect);

    void startSelectedStrokeWidthTransform();
    void startSelectedStrokeColorTransform();
    void startSelectedFillColorTransform();
    void applyPaintSetting(const PaintSettingsApplier &setting);
    void setFillColorMode(const ColorMode &colorMode);
    void setStrokeColorMode(const ColorMode &colorMode);

    void setStrokeCapStyle(const Qt::PenCapStyle &capStyle);
    void setStrokeJoinStyle(const Qt::PenJoinStyle &joinStyle);
    void setStrokeWidth(const qreal &strokeWidth);
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

    const QList<qsptr<BoundingBox>> &getContainedBoxesList() const;

    //    bool anim_nextRelFrameWithKey(const int &relFrame,
    //                                 int &nextRelFrame);
    //    bool anim_prevRelFrameWithKey(const int &relFrame,
    //                                 int &prevRelFrame);

    bool differenceInPathEffectsBetweenFrames(const int &relFrame1,
                                              const int &relFrame2) const;
    bool differenceInFillPathEffectsBetweenFrames(const int &relFrame1,
                                                  const int &relFrame2) const;
    bool differenceInOutlinePathEffectsBetweenFrames(const int &relFrame1,
                                                     const int &relFrame2) const;
    void updateAllChildPathBoxes(const UpdateReason &reason);

    void applyPathEffects(const qreal &relFrame,
                          SkPath * const srcDstPath,
                          BoundingBox * const box);

    void filterOutlinePathBeforeThickness(const qreal &relFrame,
                                          SkPath * const srcDstPath);
    void filterOutlinePath(const qreal &relFrame,
                           SkPath * const srcDstPath);
    void filterFillPath(const qreal &relFrame,
                        SkPath * const srcDstPath);

    bool diffsAffectingContainedBoxes(const int &relFrame1,
                                      const int &relFrame2);

    void deselectAllBoxesFromBoxesGroup();
    void selectAllBoxesFromBoxesGroup();
    void addContainedBoxesToSelection(const QRectF &rect);

    void setIsCurrentGroup_k(const bool &bT);
    void ungroup_k();

    bool isCurrentGroup() const;
    void addContainedBox(const qsptr<BoundingBox> &child);
    void addContainedBoxToListAt(const int &index,
                                 const qsptr<BoundingBox> &child);
    void updateContainedBoxIds(const int &firstId);
    void updateContainedBoxIds(const int &firstId,
                              const int &lastId);
    void removeContainedBox_k(const qsptr<BoundingBox> &child);
    void increaseContainedBoxZInList(BoundingBox * const child);
    void decreaseContainedBoxZInList(BoundingBox * const child);
    void bringContainedBoxToEndList(BoundingBox * const child);
    void bringContainedBoxToFrontList(BoundingBox * const child);
    void moveContainedBoxInList(BoundingBox * const child, const int &to);
    void moveContainedBoxInList(BoundingBox * const child,
                                const int &from,
                                const int &to);
    void moveContainedBoxBelow(BoundingBox * const boxToMove,
                               BoundingBox * const below);
    void moveContainedBoxAbove(BoundingBox * const boxToMove,
                               BoundingBox * const above);

    void removeContainedBoxFromList(const int &id);
    void setDescendantCurrentGroup(const bool &bT);
    bool isDescendantCurrentGroup() const;

    void readChildBoxes(QIODevice *target);

    int abstractionIdToBoxId(const int& absId) const {
        if(absId < ca_getNumberOfChildren()) return -1;
        const int revId = absId - ca_getNumberOfChildren();
        return mContainedBoxes.count() - revId - 1;
    }

    int boxIdToAbstractionId(const int& boxId) const {
        const int revId = mContainedBoxes.count() - boxId - 1;
        return revId + ca_getNumberOfChildren();
    }
    int getContainedBoxesCount() const;
    void removeAllContainedBoxes();
protected:
    void removeContainedBox(const qsptr<BoundingBox> &child);
    int getContainedBoxIndex(BoundingBox *child);

    bool mIsCurrentGroup = false;
    bool mIsDescendantCurrentGroup = false;
    QList<qsptr<BoundingBox>> mContainedBoxes;

    qsptr<PathEffectAnimators> mPathEffectsAnimators;
    qsptr<PathEffectAnimators> mFillPathEffectsAnimators;
    qsptr<PathEffectAnimators> mOutlinePathEffectsAnimators;
private:
    void iniPathEffects();
signals:
    void setParentAsCurrentGroup();
};

#endif // GROUPBOX_H
