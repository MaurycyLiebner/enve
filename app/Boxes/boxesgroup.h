#ifndef BOXESGROUP_H
#define BOXESGROUP_H
#include "Boxes/boundingbox.h"
#include "Boxes/vectorpath.h"

#define Q_FOREACHBoxInListInverted(boxesList) BoundingBox *box = getAtIndexOrGiveNull((boxesList).count() - 1, (boxesList)); \
    for(int i = (boxesList).count() - 1; i >= 0; i--, box = getAtIndexOrGiveNull(i, (boxesList)) )



class MainWindow;

class VectorPathEdge;

struct BoxesGroupRenderData : public BoundingBoxRenderData {
    friend class StdSelfRef;
    BoxesGroupRenderData(BoundingBox *parentBoxT) :
        BoundingBoxRenderData(parentBoxT) {
        mDelayDataSet = true;
    }

    void updateRelBoundingRect() {
        SkPath boundingPaths = SkPath();
        Q_FOREACH(const BoundingBoxRenderDataSPtr &child,
                  childrenRenderData) {
            SkPath childPath;
            childPath.addRect(
                    QRectFToSkRect(
                        child->relBoundingRect));
            childPath.transform(
                        QMatrixToSkMatrix(
                            child->relTransform) );
            boundingPaths.addPath(childPath);
        }
        relBoundingRect = SkRectToQRectF(boundingPaths.computeTightBounds());
    }
    void renderToImage();

    QList<BoundingBoxRenderDataSPtr> childrenRenderData;
protected:
    void drawSk(SkCanvas *canvas) {
        canvas->save();

        Q_FOREACH(const BoundingBoxRenderDataSPtr &child,
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

    void setIsCurrentGroup(const bool &bT);

    BoundingBox *getPathAtFromAllAncestors(const QPointF &absPos);

    void ungroup();

    bool isCurrentGroup();
    void addContainedBox(const BoundingBoxQSPtr &child);
    void addContainedBoxToListAt(const int &index,
                                 const BoundingBoxQSPtr &child,
                                 const bool &saveUndoRedo = true);
    void updateContainedBoxIds(const int &firstId,
                          const bool &saveUndoRedo = true);
    void updateContainedBoxIds(const int &firstId,
                          const int &lastId,
                          const bool &saveUndoRedo = true);
    void removeContainedBox(const BoundingBoxQSPtr &child);
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

    QRectF getRelBoundingRectAtRelFrame(const int &relFrame) {
        SkPath boundingPaths = SkPath();
        int absFrame = prp_relFrameToAbsFrame(relFrame);
        Q_FOREACH(const QSharedPointer<BoundingBox> &child, mContainedBoxes) {
            int childRelFrame = child->prp_absFrameToRelFrame(absFrame);
            if(child->isRelFrameVisibleAndInVisibleDurationRect(childRelFrame)) {
                SkPath childPath;
                childPath.addRect(
                        QRectFToSkRect(
                            child->getRelBoundingRectAtRelFrame(childRelFrame)));
                childPath.transform(
                            QMatrixToSkMatrix(
                                child->getTransformAnimator()->
                                    getRelativeTransformAtRelFrame(childRelFrame)) );
                boundingPaths.addPath(childPath);
            }
        }
        return SkRectToQRectF(boundingPaths.computeTightBounds());
    }

    void applyCurrentTransformation();

    bool relPointInsidePath(const QPointF &relPos);
//    QPointF getRelCenterPosition();

    void SWT_addChildrenAbstractions(SingleWidgetAbstraction *abstraction,
                                     ScrollWidgetVisiblePart *visiblePartWidget);

    bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                             const bool &parentSatisfies,
                             const bool &parentMainTarget);
    void startSelectedStrokeWidthTransform();
    void startSelectedStrokeColorTransform();
    void startSelectedFillColorTransform();

    void applyPaintSetting(
            const PaintSetting &setting) {
        Q_FOREACH(const QSharedPointer<BoundingBox> &box, mContainedBoxes) {
            box->applyPaintSetting(setting);
        }
    }

    void setFillColorMode(const ColorMode &colorMode) {
        Q_FOREACH(const QSharedPointer<BoundingBox> &box,  mContainedBoxes) {
            box->setFillColorMode(colorMode);
        }
    }
    void setStrokeColorMode(const ColorMode &colorMode) {
        Q_FOREACH(const QSharedPointer<BoundingBox> &box, mContainedBoxes) {
            box->setStrokeColorMode(colorMode);
        }
    }
    void clearAllCache();
    void drawPixmapSk(SkCanvas *canvas);
    void setDescendantCurrentGroup(const bool &bT);
    bool isDescendantCurrentGroup();
    bool shouldPaintOnImage();

    bool SWT_isBoxesGroup() { return true; }
    void drawSk(SkCanvas *canvas);
    void drawSelectedSk(SkCanvas *canvas,
                        const CanvasMode &currentCanvasMode,
                        const SkScalar &invScale);
    void prp_setAbsFrame(const int &frame);
    void schedulerProccessed();

    BoundingBoxRenderDataSPtr createRenderData() {
        return SPtrCreate(BoxesGroupRenderData)(this);
    }

    void setupBoundingBoxRenderDataForRelFrameF(const qreal &relFrame,
                                                BoundingBoxRenderData* data);

    bool prp_differencesBetweenRelFrames(const int &relFrame1,
                                         const int &relFrame2);
    bool prp_differencesBetweenRelFramesIncludingInheritedExcludingContainedBoxes(
            const int &relFrame1, const int &relFrame2);
    void prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                              int *lastIdentical,
                                              const int &relFrame);
    void getFirstAndLastIdenticalForMotionBlur(int *firstIdentical,
                                               int *lastIdentical,
                                               const int &relFrame,
                                               const bool &takeAncestorsIntoAccount = true);
    void processSchedulers();
    void addSchedulersToProcess();

    void writeBoundingBox(QIODevice *target);
    void readBoundingBox(QIODevice *target);
    void prp_setParentFrameShift(const int &shift,
                                 ComplexAnimator *parentAnimator);
    void shiftAll(const int &shift);

    int setBoxLoadId(const int &loadId) {
        int loadIdT = BoundingBox::setBoxLoadId(loadId);
        foreach(const QSharedPointer<BoundingBox> &child, mContainedBoxes) {
            loadIdT = child->setBoxLoadId(loadIdT);
        }

        return loadIdT;
    }

    virtual void clearBoxLoadId() {
        BoundingBox::clearBoxLoadId();
        foreach(const QSharedPointer<BoundingBox> &child, mContainedBoxes) {
            child->clearBoxLoadId();
        }
    }

    const QList<QSharedPointer<BoundingBox> > &getContainedBoxesList() const {
        return mContainedBoxes;
    }
    BoundingBoxQSPtr createLink();
    void readChildBoxes(QIODevice *target);

    void addPathEffect(const PathEffectQSPtr& effect);
    void addFillPathEffect(const PathEffectQSPtr& effect);
    void addOutlinePathEffect(const PathEffectQSPtr& effect);
    void removePathEffect(const PathEffectQSPtr& effect);
    void removeFillPathEffect(const PathEffectQSPtr& effect);
    void removeOutlinePathEffect(const PathEffectQSPtr& effect);

    void updateAllChildPathBoxes(const UpdateReason &reason) {
        foreach(const QSharedPointer<BoundingBox> &box,
                mContainedBoxes) {
            if(box->SWT_isPathBox()) {
                box->scheduleUpdate(reason);
            } else if(box->SWT_isBoxesGroup()) {
                static_cast<BoxesGroup*>(box.data())->updateAllChildPathBoxes(reason);
            }
        }
    }

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
    QList<PathEffectQSPtr> mGroupPathSumEffects;
    PathEffectAnimatorsQSPtr mPathEffectsAnimators;
    PathEffectAnimatorsQSPtr mFillPathEffectsAnimators;
    PathEffectAnimatorsQSPtr mOutlinePathEffectsAnimators;

    static bool mCtrlsAlwaysVisible;
    bool mIsCurrentGroup = false;
    bool mIsDescendantCurrentGroup = false;
    QList<QSharedPointer<BoundingBox> > mContainedBoxes;

    //QList<QSharedPointer<BoundingBox> > mUpdateChildrenAwaitingUpdate;
    int getContainedBoxIndex(BoundingBox *child);
    void iniPathEffects();
    bool isLastPathBox(PathBox *pathBox);
signals:
    void changeChildZSignal(int, int);
    void removeAnimatedBoundingBoxSignal(BoundingBox*);
    void addAnimatedBoundingBoxSignal(BoundingBox*);
};

#endif // BOXESGROUP_H
