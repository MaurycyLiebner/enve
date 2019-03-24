#ifndef BOXESGROUP_H
#define BOXESGROUP_H
#include "Boxes/boundingbox.h"

class PathBox;
class PathEffectAnimators;

class BoxesGroup : public BoundingBox {
    Q_OBJECT
    friend class SelfRef;
protected:
    BoxesGroup(const BoundingBoxType& type = TYPE_GROUP);
public:
    ~BoxesGroup();

    virtual BoundingBox *getBoxAt(const QPointF &absPos);

    bool SWT_isBoxesGroup() const;
    void anim_setAbsFrame(const int &frame);
    void anim_scaleTime(const int& pivotAbsFrame, const qreal& scale);
    //MovablePoint *getPointAt(const QPointF &absPos, const CanvasMode &currentMode);


    BoundingBox *getBoxAtFromAllDescendents(const QPointF &absPos);

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
    FillSettingsAnimator *getFillSettings() const;
    OutlineSettingsAnimator *getStrokeSettings() const;
    void updateAllBoxes(const UpdateReason &reason);

    QRectF getRelBoundingRectAtRelFrame(const qreal &relFrame);

    void applyCurrentTransformation();

    bool relPointInsidePath(const QPointF &relPos) const;
//    QPointF getRelCenterPosition();

    void SWT_addChildrenAbstractions(SingleWidgetAbstraction *abstraction,
                                     const UpdateFuncs &updateFuncs,
                                     const int& visiblePartWidgetId);

    bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                             const bool &parentSatisfies,
                             const bool &parentMainTarget) const;
    void startSelectedStrokeWidthTransform();
    void startSelectedStrokeColorTransform();
    void startSelectedFillColorTransform();

    void applyPaintSetting(const PaintSettingsApplier &setting);

    void setFillColorMode(const ColorMode &colorMode);
    void setStrokeColorMode(const ColorMode &colorMode);
    void drawPixmapSk(SkCanvas * const canvas,
                      GrContext * const grContext);

    stdsptr<BoundingBoxRenderData> createRenderData();

    void setupBoundingBoxRenderDataForRelFrameF(const qreal &relFrame,
                                                BoundingBoxRenderData* data);

    FrameRange prp_getIdenticalRelFrameRange(const int &relFrame) const;
    FrameRange getFirstAndLastIdenticalForMotionBlur(
            const int &relFrame, const bool &takeAncestorsIntoAccount = true);
    void scheduleWaitingTasks();
    void queScheduledTasks();

    void writeBoundingBox(QIODevice *target);
    void readBoundingBox(QIODevice *target);
    void prp_setParentFrameShift(const int &shift,
                                 ComplexAnimator *parentAnimator);
    void shiftAll(const int &shift);

    qsptr<BoundingBox> createLink();

    void addPathEffect(const qsptr<PathEffect>& effect);
    void addFillPathEffect(const qsptr<PathEffect>& effect);
    void addOutlinePathEffect(const qsptr<PathEffect>& effect);
    void removePathEffect(const qsptr<PathEffect>& effect);
    void removeFillPathEffect(const qsptr<PathEffect>& effect);
    void removeOutlinePathEffect(const qsptr<PathEffect>& effect);

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
    bool shouldPaintOnImage() const;

    void schedulerProccessed();

    bool prp_differencesBetweenRelFramesIncludingInheritedExcludingContainedBoxes(
            const int &relFrame1, const int &relFrame2);
    const QList<qsptr<BoundingBox> > &getContainedBoxesList() const;
    void readChildBoxes(QIODevice *target);

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

    void filterOutlinePathBeforeThicknessForRelFrame(const qreal &relFrame,
                                                     SkPath * const srcDstPath);
    void filterOutlinePathForRelFrame(const qreal &relFrame,
                                      SkPath * const srcDstPath);
    void filterFillPathForRelFrame(const qreal &relFrame,
                                   SkPath * const srcDstPath);
//    bool anim_nextRelFrameWithKey(const int &relFrame,
//                                 int &nextRelFrame);
//    bool anim_prevRelFrameWithKey(const int &relFrame,
//                                 int &prevRelFrame);
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
protected:
    void removeContainedBox(const qsptr<BoundingBox> &child);

    QList<qsptr<PathEffect>> mGroupPathSumEffects;
    qsptr<PathEffectAnimators> mPathEffectsAnimators;
    qsptr<PathEffectAnimators> mFillPathEffectsAnimators;
    qsptr<PathEffectAnimators> mOutlinePathEffectsAnimators;

    static bool mCtrlsAlwaysVisible;
    bool mIsCurrentGroup = false;
    bool mIsDescendantCurrentGroup = false;
    QList<qsptr<BoundingBox> > mContainedBoxes;

    //QList<qsptr<BoundingBox> > mUpdateChildrenAwaitingUpdate;
    int getContainedBoxIndex(BoundingBox *child);
    void iniPathEffects();
    bool isLastPathBox(PathBox * const pathBox);
signals:
    void setParentAsCurrentGroup();
    void changeChildZSignal(int, int);
    void removeAnimatedBoundingBoxSignal(BoundingBox*);
    void addAnimatedBoundingBoxSignal(BoundingBox*);
};

#endif // BOXESGROUP_H
