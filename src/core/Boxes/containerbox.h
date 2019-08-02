#ifndef CONTAINERBOX_H
#define CONTAINERBOX_H
#include "boundingbox.h"
#include "conncontext.h"
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

    bool SWT_dropSupport(const QMimeData* const data);
    bool SWT_dropIntoSupport(const int index, const QMimeData* const data);
    bool SWT_drop(const QMimeData* const data);
    bool SWT_dropInto(const int index, const QMimeData* const data);

    void drawContained(SkCanvas * const canvas);
    void drawPixmapSk(SkCanvas * const canvas);

    qsptr<BoundingBox> createLink();
    stdsptr<BoxRenderData> createRenderData();
    void setupRenderData(const qreal relFrame, BoxRenderData * const data);

    virtual BoundingBox *getBoxAt(const QPointF &absPos);

    bool unboundChildren() const;
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
    void setupCanvasMenu(PropertyMenu * const menu);

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

    void strokeWidthAction(const QrealAction& action);

    void startSelectedStrokeColorTransform();
    void startSelectedFillColorTransform();
    void applyPaintSetting(const PaintSettingsApplier &setting);

    void setStrokeCapStyle(const SkPaint::Cap capStyle);
    void setStrokeJoinStyle(const SkPaint::Join joinStyle);

    FillSettingsAnimator *getFillSettings() const;
    OutlineSettingsAnimator *getStrokeSettings() const;

    void scheduleChildWaitingTasks();
    void scheduleWaitingTasks();
    void queChildScheduledTasks();
    void queScheduledTasks();


    void writeBoundingBox(QIODevice * const target);
    void readBoundingBox(QIODevice * const target);

    void promoteToLayer();
    void demoteToGroup();

    const ConnContextObjList<qsptr<BoundingBox>> &getContainedBoxes() const;

    bool differenceInPathEffectsBetweenFrames(const int relFrame1,
                                              const int relFrame2) const;
    bool differenceInFillPathEffectsBetweenFrames(const int relFrame1,
                                                  const int relFrame2) const;
    bool differenceInOutlinePathEffectsBetweenFrames(const int relFrame1,
                                                     const int relFrame2) const;
    void updateAllChildPathBoxes(const UpdateReason reason);

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
    void insertContainedBox(const int id, const qsptr<BoundingBox> &child);
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
        return absId - ca_getNumberOfChildren();
    }

    int boxIdToAbstractionId(const int boxId) const {
        return boxId + ca_getNumberOfChildren();
    }
    int getContainedBoxesCount() const;
    void removeAllContainedBoxes();

    void updateIfUsesProgram(const ShaderEffectProgram * const program) const final;
protected:
    void removeContainedBox(const qsptr<BoundingBox> &child);

    bool mIsCurrentGroup = false;
    bool mIsDescendantCurrentGroup = false;
    ConnContextObjList<qsptr<BoundingBox>> mContainedBoxes;

    qsptr<PathEffectAnimators> mPathEffectsAnimators;
    qsptr<PathEffectAnimators> mFillPathEffectsAnimators;
    qsptr<PathEffectAnimators> mOutlineBasePathEffectsAnimators;
    qsptr<PathEffectAnimators> mOutlinePathEffectsAnimators;
private:
    void setupLayerRenderData(const qreal relFrame,
                              BoxRenderData * const data);
    void iniPathEffects();
};

#endif // CONTAINERBOX_H
