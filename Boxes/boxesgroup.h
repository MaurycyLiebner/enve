#ifndef BOXESGROUP_H
#define BOXESGROUP_H
#include "Boxes/boundingbox.h"
#include "Boxes/vectorpath.h"
#include "fillstrokesettings.h"

#define Q_FOREACHBoxInListInverted(boxesList) BoundingBox *box = getAtIndexOrGiveNull((boxesList).count() - 1, (boxesList)); \
    for(int i = (boxesList).count() - 1; i >= 0; i--, box = getAtIndexOrGiveNull(i, (boxesList)) )



class MainWindow;

class VectorPathEdge;

struct BoxesGroupRenderData : public BoundingBoxRenderData {
    BoxesGroupRenderData(BoundingBox *parentBoxT) :
        BoundingBoxRenderData(parentBoxT) {
        mDelayDataSet = true;
    }
    QList<std::shared_ptr<BoundingBoxRenderData> > childrenRenderData;

    void updateRelBoundingRect() {
        SkPath boundingPaths = SkPath();
        Q_FOREACH(const std::shared_ptr<BoundingBoxRenderData> &child,
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
protected:
    void drawSk(SkCanvas *canvas) {
        canvas->save();

        Q_FOREACH(const std::shared_ptr<BoundingBoxRenderData> &child,
                  childrenRenderData) {
            child->drawRenderedImageForParent(canvas);
        }

        canvas->restore();
    }
};

class BoxesGroup : public BoundingBox {
    Q_OBJECT
public:
    BoxesGroup();
    BoxesGroup(FillStrokeSettingsWidget *fillStrokeSetting);
    ~BoxesGroup();

    void deselectAllBoxesFromBoxesGroup();
    void selectAllBoxesFromBoxesGroup();

    BoundingBox *getBoxAt(const QPointF &absPos);
    //MovablePoint *getPointAt(const QPointF &absPos, const CanvasMode &currentMode);

    void addContainedBoxesToSelection(const QRectF &rect);

    void setIsCurrentGroup(const bool &bT);

    BoundingBox *getPathAtFromAllAncestors(const QPointF &absPos);

    void setCurrentFillStrokeSettingsFromBox(BoundingBox *box);

    void ungroup();

    bool isCurrentGroup();
    void addContainedBox(BoundingBox *child);
    void addContainedBoxToListAt(const int &index, BoundingBox *child,
                          const bool &saveUndoRedo = true);
    void updateContainedBoxIds(const int &firstId,
                          const bool &saveUndoRedo = true);
    void updateContainedBoxIds(const int &firstId,
                          const int &lastId,
                          const bool &saveUndoRedo = true);
    void removeContainedBox(BoundingBox *child);
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
    void updateAllBoxes();

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

    void makeDuplicate(Property *targetBox);
    BoundingBox *createNewDuplicate();

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

    BoundingBoxRenderData *createRenderData() {
        return new BoxesGroupRenderData(this);
    }

    void setupBoundingBoxRenderDataForRelFrame(const int &relFrame,
                                               BoundingBoxRenderData *data);

    bool prp_differencesBetweenRelFrames(const int &relFrame1,
                                         const int &relFrame2);
    void prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                               int *lastIdentical,
                                               const int &relFrame);
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

    const QList<QSharedPointer<BoundingBox> > &getChildBoxesList() const {
        return mContainedBoxes;
    }
    BoundingBox *createLink();
    void readChildBoxes(QIODevice *target);

    void addPathEffect(PathEffect *effect);
    void addFillPathEffect(PathEffect *effect);
    void addOutlinePathEffect(PathEffect *effect);
    void removePathEffect(PathEffect *effect);
    void removeFillPathEffect(PathEffect *effect);
    void removeOutlinePathEffect(PathEffect *effect);

    void updateAllChildPathBoxes() {
        foreach(const QSharedPointer<BoundingBox> &box,
                mContainedBoxes) {
            if(box->SWT_isPathBox()) {
                box->scheduleUpdate();
            } else if(box->SWT_isBoxesGroup()) {
                ((BoxesGroup*)box.data())->updateAllChildPathBoxes();
            }
        }
    }

    void filterPathForRelFrame(const int &relFrame,
                               SkPath *srcDstPath);
    void filterOutlinePathBeforeThicknessForRelFrame(
            const int &relFrame, SkPath *srcDstPath);
    void filterOutlinePathForRelFrame(const int &relFrame,
                               SkPath *srcDstPath);
    void filterFillPathForRelFrame(const int &relFrame,
                                   SkPath *srcDstPath);
//    bool prp_nextRelFrameWithKey(const int &relFrame,
//                                 int &nextRelFrame);
//    bool prp_prevRelFrameWithKey(const int &relFrame,
//                                 int &prevRelFrame);
protected:
    PathEffectAnimatorsQSPtr mPathEffectsAnimators;
    PathEffectAnimatorsQSPtr mFillPathEffectsAnimators;
    PathEffectAnimatorsQSPtr mOutlinePathEffectsAnimators;

    static bool mCtrlsAlwaysVisible;
    FillStrokeSettingsWidget *mFillStrokeSettingsWidget;
    bool mIsCurrentGroup = false;
    bool mIsDescendantCurrentGroup = false;
    QList<QSharedPointer<BoundingBox> > mContainedBoxes;

    //QList<QSharedPointer<BoundingBox> > mUpdateChildrenAwaitingUpdate;
    int getChildBoxIndex(BoundingBox *child);
    void iniPathEffects();
signals:
    void changeChildZSignal(int, int);
    void removeAnimatedBoundingBoxSignal(BoundingBox*);
    void addAnimatedBoundingBoxSignal(BoundingBox*);
};

#endif // BOXESGROUP_H
