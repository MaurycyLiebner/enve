#ifndef CONTAINERBOX_H
#define CONTAINERBOX_H
#include "boundingbox.h"
#include "conncontext.h"
class PathBox;
class PathEffectAnimators;

class ContainerBox : public BoundingBox {
    e_OBJECT
protected:
    ContainerBox(const eBoxType type);
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

    bool shouldScheduleUpdate() { return !SWT_isGroupBox(); }

    void queChildScheduledTasks();
    void clearRenderData();
    void queScheduledTasks();

    void writeAllContained(QIODevice * const dst);
    void writeBoundingBox(QIODevice * const dst);
    void readContained(QIODevice * const src);
    void readAllContained(QIODevice * const src);
    void readBoundingBox(QIODevice * const src);

    void promoteToLayer();
    void demoteToGroup();

    const QList<BoundingBox *> &getContainedBoxes() const;

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

    void updateContainedBoxes();
    bool replaceContained(const qsptr<eBoxOrSound>& replaced,
                             const qsptr<eBoxOrSound>& replacer);
    void addContained(const qsptr<eBoxOrSound> &child);
    void insertContained(const int id, const qsptr<eBoxOrSound> &child);
    void updateContainedIds(const int firstId);
    void updateContainedIds(const int firstId,
                              const int lastId);
    int getContainedIndex(eBoxOrSound * const child);
    qsptr<eBoxOrSound> takeContained_k(const int id);
    void removeContained_k(const qsptr<eBoxOrSound> &child);
    void increaseContainedZInList(eBoxOrSound * const child);
    void decreaseContainedZInList(eBoxOrSound * const child);
    void bringContainedToEndList(eBoxOrSound * const child);
    void bringContainedToFrontList(eBoxOrSound * const child);
    void moveContainedInList(eBoxOrSound * const child, const int to);
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
    int getContainedBoxesCount() const;
    void removeAllContained();

    void updateIfUsesProgram(const ShaderEffectProgram * const program) const final;
private:
    void iniPathEffects();
    void updateRelBoundingRect();
protected:
    void removeContained(const qsptr<eBoxOrSound> &child);

    bool mIsCurrentGroup = false;
    bool mIsDescendantCurrentGroup = false;
    QList<BoundingBox*> mContainedBoxes;
    ConnContextObjList<qsptr<eBoxOrSound>> mContained;

    qsptr<PathEffectAnimators> mPathEffectsAnimators;
    qsptr<PathEffectAnimators> mFillPathEffectsAnimators;
    qsptr<PathEffectAnimators> mOutlineBasePathEffectsAnimators;
    qsptr<PathEffectAnimators> mOutlinePathEffectsAnimators;
};

#endif // CONTAINERBOX_H
