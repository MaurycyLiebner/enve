#ifndef BOXESGROUP_H
#define BOXESGROUP_H
#include "Boxes/boundingbox.h"

#define Q_FOREACHBoxInListInverted(boxesList) BoundingBox *box = getAtIndexOrGiveNull((boxesList).count() - 1, (boxesList)); \
    for(int i = (boxesList).count() - 1; i >= 0; i--, box = getAtIndexOrGiveNull(i, (boxesList)) )



class PathBox;
class PathEffectAnimators;

struct BoxesGroupRenderData : public BoundingBoxRenderData {
    friend class StdSelfRef;
    BoxesGroupRenderData(BoundingBox *parentBoxT) :
        BoundingBoxRenderData(parentBoxT) {
        mDelayDataSet = true;
    }

    void updateRelBoundingRect() {
        SkPath boundingPaths = SkPath();
        Q_FOREACH(const stdsptr<BoundingBoxRenderData> &child,
                  childrenRenderData) {
            SkPath childPath;
            childPath.addRect(
                    QRectFToSkRect(
                        child->fRelBoundingRect));
            childPath.transform(
                        QMatrixToSkMatrix(
                            child->fRelTransform) );
            boundingPaths.addPath(childPath);
        }
        fRelBoundingRect = SkRectToQRectF(boundingPaths.computeTightBounds());
    }
    void renderToImage();

    QList<stdsptr<BoundingBoxRenderData>> childrenRenderData;
protected:
    void drawSk(SkCanvas *canvas) {
        canvas->save();

        Q_FOREACH(const stdsptr<BoundingBoxRenderData> &child,
                  childrenRenderData) {
            child->drawRenderedImageForParent(canvas);
        }

        canvas->restore();
    }
};

class BoxesGroup : public BoundingBox {
    Q_OBJECT
    friend class SelfRef;
public:
    BoxesGroup(const BoundingBoxType& type = TYPE_GROUP);
    ~BoxesGroup();

    void scaleTime(const int& pivotAbsFrame, const qreal& scale);

    void deselectAllBoxesFromBoxesGroup();
    void selectAllBoxesFromBoxesGroup();

    virtual BoundingBox *getBoxAt(const QPointF &absPos);
    //MovablePoint *getPointAt(const QPointF &absPos, const CanvasMode &currentMode);

    void addContainedBoxesToSelection(const QRectF &rect);

    void setIsCurrentGroup_k(const bool &bT);

    BoundingBox *getPathAtFromAllAncestors(const QPointF &absPos);

    void ungroup_k();

    bool isCurrentGroup();
    void addContainedBox(const qsptr<BoundingBox> &child);
    void addContainedBoxToListAt(const int &index,
                                 const qsptr<BoundingBox> &child,
                                 const bool &saveUndoRedo = true);
    void updateContainedBoxIds(const int &firstId,
                          const bool &saveUndoRedo = true);
    void updateContainedBoxIds(const int &firstId,
                          const int &lastId,
                          const bool &saveUndoRedo = true);
    void removeContainedBox_k(const qsptr<BoundingBox> &child);
    void removeContainedBox(const qsptr<BoundingBox> &child);
    void increaseContainedBoxZInList(BoundingBox *child);
    void decreaseContainedBoxZInList(BoundingBox *child);
    void bringContainedBoxToEndList(BoundingBox *child);
    void bringContainedBoxToFrontList(BoundingBox *child);
    void moveContainedBoxInList(BoundingBox *child,
                         const int &from,
                         const int &to,
                         const bool &saveUndoRedo = true);
    void moveContainedBoxBelow(BoundingBox *boxToMove,
                               BoundingBox *below);
    void moveContainedBoxAbove(BoundingBox *boxToMove,
                        BoundingBox *above);

    void removeContainedBoxFromList(const int &id,
                             const bool &saveUndoRedo = true);

    void setStrokeCapStyle(const Qt::PenCapStyle &capStyle);
    void setStrokeJoinStyle(const Qt::PenJoinStyle &joinStyle);
    void setStrokeWidth(const qreal &strokeWidth, const bool &finish);

    PaintSettings *getFillSettings();
    StrokeSettings *getStrokeSettings();
    void updateAllBoxes(const UpdateReason &reason);

    QRectF getRelBoundingRectAtRelFrame(const int &relFrame);

    void applyCurrentTransformation();

    bool relPointInsidePath(const QPointF &relPos);
//    QPointF getRelCenterPosition();

    void SWT_addChildrenAbstractions(SingleWidgetAbstraction *abstraction,
                                     const UpdateFuncs &updateFuncs,
                                     const int& visiblePartWidgetId);

    bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                             const bool &parentSatisfies,
                             const bool &parentMainTarget);
    void startSelectedStrokeWidthTransform();
    void startSelectedStrokeColorTransform();
    void startSelectedFillColorTransform();

    void applyPaintSetting(PaintSetting *setting);

    void setFillColorMode(const ColorMode &colorMode);
    void setStrokeColorMode(const ColorMode &colorMode);
    void clearAllCache();
    void drawPixmapSk(SkCanvas *canvas, GrContext * const grContext);
    void setDescendantCurrentGroup(const bool &bT);
    bool isDescendantCurrentGroup();
    bool shouldPaintOnImage();

    bool SWT_isBoxesGroup();
    void drawSk(SkCanvas *canvas);
    void drawSelectedSk(SkCanvas *canvas,
                        const CanvasMode &currentCanvasMode,
                        const SkScalar &invScale);
    void prp_setAbsFrame(const int &frame);
    void schedulerProccessed();

    stdsptr<BoundingBoxRenderData> createRenderData();

    void setupBoundingBoxRenderDataForRelFrameF(const qreal &relFrame,
                                                BoundingBoxRenderData* data);

    bool prp_differencesBetweenRelFrames(const int &relFrame1,
                                         const int &relFrame2);
    bool prp_differencesBetweenRelFramesIncludingInheritedExcludingContainedBoxes(
            const int &relFrame1, const int &relFrame2);
    FrameRange prp_getFirstAndLastIdenticalRelFrame(const int &relFrame);
    FrameRange getFirstAndLastIdenticalForMotionBlur(
            const int &relFrame, const bool &takeAncestorsIntoAccount = true);
    void scheduleWaitingTasks();
    void queScheduledTasks();

    void writeBoundingBox(QIODevice *target);
    void readBoundingBox(QIODevice *target);
    void prp_setParentFrameShift(const int &shift,
                                 ComplexAnimator *parentAnimator);
    void shiftAll(const int &shift);

    int setBoxLoadId(const int &loadId);

    virtual void clearBoxLoadId();

    const QList<qsptr<BoundingBox> > &getContainedBoxesList() const;
    qsptr<BoundingBox> createLink();
    void readChildBoxes(QIODevice *target);

    void addPathEffect(const qsptr<PathEffect>& effect);
    void addFillPathEffect(const qsptr<PathEffect>& effect);
    void addOutlinePathEffect(const qsptr<PathEffect>& effect);
    void removePathEffect(const qsptr<PathEffect>& effect);
    void removeFillPathEffect(const qsptr<PathEffect>& effect);
    void removeOutlinePathEffect(const qsptr<PathEffect>& effect);

    void updateAllChildPathBoxes(const UpdateReason &reason);

    void filterPathForRelFrame(const int &relFrame,
                               SkPath *srcDstPath,
                               BoundingBox *box);
    void filterPathForRelFrameUntilGroupSum(const int &relFrame,
                                            SkPath *srcDstPath,
                                            BoundingBox *box);
    void filterOutlinePathBeforeThicknessForRelFrame(const int &relFrame,
                                                     SkPath *srcDstPath,
                                                     BoundingBox *box);
    void filterOutlinePathForRelFrame(const int &relFrame,
                                      SkPath *srcDstPath);
    void filterFillPathForRelFrame(const int &relFrame,
                                   SkPath *srcDstPath);

    void filterPathForRelFrameF(const qreal &relFrame,
                               SkPath *srcDstPath,
                               BoundingBox *box);
    void filterPathForRelFrameUntilGroupSumF(const qreal &relFrame,
                                            SkPath *srcDstPath);
    void filterOutlinePathBeforeThicknessForRelFrameF(const qreal &relFrame,
                                                     SkPath *srcDstPath);
    void filterOutlinePathForRelFrameF(const qreal &relFrame,
                                      SkPath *srcDstPath);
    void filterFillPathForRelFrameF(const qreal &relFrame,
                                   SkPath *srcDstPath);
//    bool prp_nextRelFrameWithKey(const int &relFrame,
//                                 int &nextRelFrame);
//    bool prp_prevRelFrameWithKey(const int &relFrame,
//                                 int &prevRelFrame);
    bool enabledGroupPathSumEffectPresent();
protected:
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
    bool isLastPathBox(PathBox *pathBox);
signals:
    void setParentAsCurrentGroup();
    void changeChildZSignal(int, int);
    void removeAnimatedBoundingBoxSignal(BoundingBox*);
    void addAnimatedBoundingBoxSignal(BoundingBox*);
};

#endif // BOXESGROUP_H
