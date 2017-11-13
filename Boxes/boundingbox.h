#ifndef CHILDPARENT_H
#define CHILDPARENT_H
#include <QMatrix>
#include "transformable.h"
#include "fillstrokesettings.h"
#include "Animators/transformanimator.h"

#include "PixmapEffects/pixmapeffect.h"

#include "Animators/effectanimators.h"

#include "BoxesList/OptimalScrollArea/singlewidgettarget.h"
#include <unordered_map>

#include "boundingboxrendercontainer.h"
#include "skiaincludes.h"
#include "updatable.h"

class Canvas;

class MovablePoint;

class NodePoint;
class PathEffect;
class PathAnimator;
class DurationRectangleMovable;

enum CanvasMode : short;

enum BoundingBoxType {
    TYPE_VECTOR_PATH,
    TYPE_CIRCLE,
    TYPE_IMAGE,
    TYPE_RECTANGLE,
    TYPE_TEXT,
    TYPE_GROUP,
    TYPE_CANVAS,
    TYPE_INTERNAL_LINK,
    TYPE_INTERNAL_LINK_CANVAS,
    TYPE_EXTERNAL_LINK,
    TYPE_PARTICLES,
    TYPE_VIDEO,
    TYPE_IMAGESQUENCE,
    TYPE_PAINT,
    TYPE_BONE
};

class BoxesGroup;

class VectorPathEdge;

class VectorPath;

class DurationRectangle;

struct FunctionWaitingForBoxLoad {
    FunctionWaitingForBoxLoad(const int &boxIdT) {
        loadBoxId = boxIdT;
    }
    virtual ~FunctionWaitingForBoxLoad() {}

    virtual void boxLoaded(BoundingBox *box) = 0;
    int loadBoxId;
};

class BoundingBoxMimeData : public QMimeData {
    Q_OBJECT
public:
    BoundingBoxMimeData(BoundingBox *target) : QMimeData() {
        mBoundingBox = target;
    }

    BoundingBox *getBoundingBox() {
        return mBoundingBox;
    }

    bool hasFormat(const QString &mimetype) const {
        if(mimetype == "boundingbox") return true;
        return false;
    }
private:
    BoundingBox *mBoundingBox;
};

struct BoundingBoxRenderData : public Updatable {
    BoundingBoxRenderData(BoundingBox *parentBoxT);

    virtual ~BoundingBoxRenderData();
    bool renderedToImage = false;
    QMatrix transform;
    QMatrix relTransform;
    QRectF relBoundingRect;
    qreal opacity;
    qreal resolution;
    qreal effectsMargin;
    int relFrame;
    QList<PixmapEffectRenderData*> pixmapEffects;
    SkPoint drawPos = SkPoint::Make(0.f, 0.f);
    SkBlendMode blendMode = SkBlendMode::kSrcOver;
    QRectF maxBoundsRect;

    QWeakPointer<BoundingBox> parentBox;

    virtual void updateRelBoundingRect();
    void drawRenderedImageForParent(SkCanvas *canvas);
    virtual void renderToImage();
    sk_sp<SkImage> renderedImage;

    void processUpdate();

    void beforeUpdate();

    void afterUpdate();

    void schedulerProccessed();

    void addSchedulerNow();

    virtual bool allDataReady() { return true; }

    void dataSet();

    void clearPixmapEffects() {
        pixmapEffects.clear();
        effectsMargin = 0.;
    }

    virtual QPointF getCenterPosition() {
        return relBoundingRect.center();
    }
protected:
    bool mDelayDataSet = false;
    bool mDataSet = false;
    virtual void drawSk(SkCanvas *canvas) = 0;
};

class BoundingBox :
        public ComplexAnimator {
    Q_OBJECT
public:
    BoundingBox(const BoundingBoxType &type);
    virtual ~BoundingBox();

    virtual BoundingBox *createLink();
    virtual BoundingBox *createLinkForLinkGroup() {
        BoundingBox *box = createLink();
        box->clearEffects();
        return box;
    }

    void clearEffects() {
        mEffectsAnimators->ca_removeAllChildAnimators();
    }

    virtual void setFont(const QFont &) {}
    virtual void setSelectedFontSize(const qreal &) {}
    virtual void setSelectedFontFamilyAndStyle(const QString &,
                                               const QString &) {}

    virtual QPointF getRelCenterPosition() {
        return mRelBoundingRect.center();
    }

    virtual void centerPivotPosition(const bool &saveUndoRedo = false) {
        mTransformAnimator->setPivotWithoutChangingTransformation(
                    getRelCenterPosition(), saveUndoRedo);
    }
    virtual void setPivotPosition(
            const QPointF &pos,
            const bool &saveUndoRedo = false) {
        mTransformAnimator->setPivotWithoutChangingTransformation(
                    pos, saveUndoRedo);
    }
    virtual bool isContainedIn(const QRectF &absRect);

    virtual void drawPixmapSk(SkCanvas *canvas);
    virtual void drawPixmapSk(SkCanvas *canvas, SkPaint *paint);
    virtual void drawSelectedSk(SkCanvas *canvas,
                                const CanvasMode &currentCanvasMode,
                                const SkScalar &invScale);


    void applyTransformation(BoxTransformAnimator *transAnimator);

    QPointF getAbsolutePos();

    virtual void updateCombinedTransform();
    void updateCombinedTransformAfterFrameChange();

    virtual void moveByRel(const QPointF &trans);

    virtual void startTransform();
    virtual void finishTransform();

    virtual bool relPointInsidePath(const QPointF &);
    bool absPointInsidePath(const QPointF &absPos);
    virtual MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                                     const CanvasMode &currentCanvasMode,
                                     const qreal &canvasScaleInv);

    void moveUp();
    void moveDown();
    void bringToFront();
    void bringToEnd();

    void setZListIndex(const int &z,
                       const bool &saveUndoRedo = true);

    virtual void selectAndAddContainedPointsToList(const QRectF &,
                                                   QList<MovablePoint*> *) {}

    QPointF getPivotAbsPos();
    virtual void select();
    void deselect();
    int getZIndex();
    virtual void drawBoundingRectSk(SkCanvas *canvas,
                                    const qreal &invScale);

    virtual void setParentGroup(BoxesGroup *parent);
    virtual void setParent(BoundingBox *parent);
    void clearParent();

    BoxesGroup *getParentGroup();
    BoundingBox *getParent();

    virtual BoundingBox *getPathAtFromAllAncestors(const QPointF &absPos);

    virtual PaintSettings *getFillSettings();
    virtual StrokeSettings *getStrokeSettings();

    void setPivotAbsPos(const QPointF &absPos,
                        const bool &saveUndoRedo = true,
                        const bool &pivotChanged = true);

    void setPivotRelPos(const QPointF &relPos,
                        const bool &saveUndoRedo = true,
                        const bool &pivotAutoAdjust = true);

    virtual void cancelTransform();
    virtual void scale(const qreal &scaleXBy, const qreal &scaleYBy);

    virtual NodePoint *createNewPointOnLineNear(const QPointF &absPos,
                                                const bool &adjust,
                                                const qreal &canvasScaleInv) {
        Q_UNUSED(absPos);
        Q_UNUSED(adjust);
        Q_UNUSED(canvasScaleInv);
        return NULL;
    }
    virtual void saveTransformPivotAbsPos(const QPointF &absPivot);

    void setName(const QString &name);
    QString getName();

    void hide();
    void show();
    bool isVisible();
    void setVisibile(const bool &visible,
                     const bool &saveUndoRedo = true);
    void switchVisible();
    bool isParentLinkBox();
    void lock();
    void unlock();
    void setLocked(const bool &bt);
    bool isLocked();
    bool isVisibleAndUnlocked();
    virtual void rotateBy(const qreal &rot);
    virtual void scale(const qreal &scaleBy);

    void rotateRelativeToSavedPivot(const qreal &rot);
    void scaleRelativeToSavedPivot(const qreal &scaleBy);

    virtual void startPosTransform();
    virtual void startRotTransform();
    virtual void startScaleTransform();
    virtual void prp_setAbsFrame(const int &frame);

    virtual void startAllPointsTransform() {}
    virtual void finishAllPointsTransform() {}

    virtual void setStrokeCapStyle(const Qt::PenCapStyle &capStyle) {
        Q_UNUSED(capStyle); }
    virtual void setStrokeJoinStyle(const Qt::PenJoinStyle &joinStyle) {
        Q_UNUSED(joinStyle); }
    virtual void setStrokeWidth(const qreal &strokeWidth,
                                const bool &finish) {
        Q_UNUSED(strokeWidth); Q_UNUSED(finish); }

    virtual void startSelectedStrokeWidthTransform() {}
    virtual void startSelectedStrokeColorTransform() {}
    virtual void startSelectedFillColorTransform() {}

    virtual VectorPathEdge *getEdge(const QPointF &absPos,
                                    const qreal &canvasScaleInv) {
        Q_UNUSED(absPos);
        Q_UNUSED(canvasScaleInv);
        return NULL;
    }
    void setAbsolutePos(const QPointF &pos,
                        const bool &saveUndoRedo = false);
    void setRelativePos(const QPointF &relPos,
                        const bool &saveUndoRedo = false);


    void scaleRelativeToSavedPivot(const qreal &scaleXBy,
                                   const qreal &scaleYBy);
    void resetScale();
    void resetTranslation();
    void resetRotation();
    BoxTransformAnimator *getTransformAnimator();
    void disablePivotAutoAdjust();
    void enablePivotAutoAdjust();

    virtual VectorPath *objectToPath() { return NULL; }
    virtual VectorPath *strokeToPath() { return NULL; }

    void updatePrettyPixmap();

    void saveOldPixmap();

    void saveUglyPaintTransform();
    void drawAsBoundingRectSk(SkCanvas *canvas,
                              const SkPath &path,
                              const SkScalar &invScale,
                              const bool &dashes);

    void redoUpdate();
    bool shouldRedoUpdate();
    void setRedoUpdateToFalse();


    void updateRelativeTransformTmp();

    virtual QPointF mapAbsPosToRel(const QPointF &absPos);
    void addEffect(PixmapEffect *effect);
    void removeEffect(PixmapEffect *effect);

    void setBlendModeSk(const SkBlendMode &blendMode);
    virtual const SkBlendMode &getBlendMode() {
        return mBlendModeSk;
    }

    virtual void updateAllBoxes();
    void selectionChangeTriggered(const bool &shiftPressed);

    bool isAnimated() { return prp_isDescendantRecording(); }

    virtual const SkPath &getRelBoundingRectPath();
    virtual QMatrix getRelativeTransformAtCurrentFrame();
    QPointF mapRelPosToAbs(const QPointF &relPos) const;

    QRectF getRelBoundingRect() const {
        return mRelBoundingRect;
    }

    virtual QRectF getRelBoundingRectAtRelFrame(const int &relFrame) {
        Q_UNUSED(relFrame);
        return getRelBoundingRect();
    }

    virtual void applyCurrentTransformation() {}

    virtual Canvas *getParentCanvas();
    virtual void reloadCacheHandler() { clearAllCache(); }

    void duplicateTransformAnimatorFrom(BoxTransformAnimator *source);

    bool SWT_isBoundingBox() { return true; }

    SingleWidgetAbstraction *SWT_getAbstractionForWidget(
            ScrollWidgetVisiblePart *visiblePartWidget);

    bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                             const bool &parentSatisfies,
                             const bool &parentMainTarget);

    bool SWT_visibleOnlyIfParentDescendant() {
        return false;
    }

    void SWT_addToContextMenu(QMenu *menu);
    bool SWT_handleContextMenuActionSelected(QAction *selectedAction);

    QMimeData *SWT_createMimeData() {
        return new BoundingBoxMimeData(this);
    }

    bool isAncestor(BoxesGroup *box) const;
    bool isAncestor(BoundingBox *box) const;
    void removeFromParent();
    void removeFromSelection();
    virtual void moveByAbs(const QPointF &trans);
    virtual void makeDuplicate(Property *property);
    Property *makeDuplicate();
    BoundingBox *createDuplicate();
    virtual BoundingBox *createNewDuplicate() = 0;
    BoundingBox *createDuplicateWithSameParent();

    virtual void drawHoveredSk(SkCanvas *canvas,
                               const SkScalar &invScale) {
        drawHoveredPathSk(canvas, mSkRelBoundingRectPath, invScale);
    }

    void drawHoveredPathSk(SkCanvas *canvas,
                           const SkPath &path,
                           const qreal &invScale);

    virtual void applyPaintSetting(
            const PaintSetting &setting) {
        Q_UNUSED(setting);
    }

    virtual void setFillColorMode(const ColorMode &colorMode) {
        Q_UNUSED(colorMode);
    }
    virtual void setStrokeColorMode(const ColorMode &colorMode) {
        Q_UNUSED(colorMode);
    }
    void switchLocked();

    void applyEffectsSk(const SkBitmap &im, const qreal &scale = 1.);

    virtual QMatrix getCombinedTransform() const;

    virtual void updateCombinedTransformTmp();
    void updateRelativeTransformAfterFrameChange();
    QPainter::CompositionMode getCompositionMode();
    bool isParticleBox();
    DurationRectangleMovable *anim_getRectangleMovableAtPos(
                                    const qreal &relX,
                                    const int &minViewedFrame,
                                    const qreal &pixelsPerFrame);

    int prp_getParentFrameShift() const;

    void setDurationRectangle(DurationRectangle *durationRect);

    bool isVisibleAndInVisibleDurationRect();
    void incUsedAsTarget();
    void decUsedAsTarget();
    void ca_childAnimatorIsRecordingChanged();

    virtual void clearAllCache();

    void startPivotTransform();
    void finishPivotTransform();
    bool hasDurationRectangle();
    void createDurationRectangle();
    void prp_drawKeys(QPainter *p,
                      const qreal &pixelsPerFrame,
                      const qreal &drawY,
                      const int &startFrame,
                      const int &endFrame);
    virtual void addPathEffect(PathEffect *) {}
    virtual void addFillPathEffect(PathEffect *) {}
    virtual void addOutlinePathEffect(PathEffect *) {}
    virtual void removePathEffect(PathEffect *) {}
    virtual void removeFillPathEffect(PathEffect *) {}
    virtual void removeOutlinePathEffect(PathEffect *) {}

    virtual void addActionsToMenu(QMenu *) {}
    virtual bool handleSelectedCanvasAction(QAction *) {
        return false;
    }

    virtual void setupBoundingBoxRenderDataForRelFrame(
            const int &relFrame, BoundingBoxRenderData *data);

    virtual BoundingBoxRenderData *createRenderData() { return NULL; }

    BoundingBoxRenderData *getCurrentRenderData();
    virtual qreal getEffectsMarginAtRelFrame(const int &relFrame);

    bool prp_differencesBetweenRelFrames(const int &relFrame1,
                                         const int &relFrame2);
    virtual void renderDataFinished(BoundingBoxRenderData *renderData);
    void updateRelBoundingRectFromRenderData(BoundingBoxRenderData *renderData);

    virtual void updateCurrentPreviewDataFromRenderData(
            BoundingBoxRenderData *renderData);
    virtual bool shouldScheduleUpdate() {
        if(mParentGroup == NULL) return false;
        if((isVisibleAndInVisibleDurationRect()) ||
           (isRelFrameInVisibleDurationRect(anim_mCurrentRelFrame))) {
            return true;
        }
        return false;
    }

    void updateCurrentRenderData();
    void nullifyCurrentRenderData();
    virtual bool isRelFrameInVisibleDurationRect(const int &relFrame);
    bool isRelFrameVisibleAndInVisibleDurationRect(const int &relFrame);
    void prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                              int *lastIdentical,
                                              const int &relFrame);
    virtual void processSchedulers();
    void addScheduler(Updatable *updatable);
    virtual void addSchedulersToProcess();

    const int &getLoadId() {
        return mLoadId;
    }

    virtual int setBoxLoadId(const int &loadId) {
        mLoadId = loadId;
        return loadId + 1;
    }

    virtual void clearBoxLoadId() {
        mLoadId = -1;
    }

    static BoundingBox *getLoadedBoxById(const int &loadId) {
        foreach(BoundingBox *box, mLoadedBoxes) {
            if(box->getLoadId() == loadId) {
                return box;
            }
        }
        return NULL;
    }

    static void addFunctionWaitingForBoxLoad(FunctionWaitingForBoxLoad *func) {
        mFunctionsWaitingForBoxLoad << func;
    }

    static void addLoadedBox(BoundingBox *box) {
        mLoadedBoxes << box;
        for(int i = 0; i < mFunctionsWaitingForBoxLoad.count(); i++) {
            FunctionWaitingForBoxLoad *funcT =
                    mFunctionsWaitingForBoxLoad.at(i);
            if(funcT->loadBoxId == box->getLoadId()) {
                funcT->boxLoaded(box);
                delete funcT;
                mFunctionsWaitingForBoxLoad.removeAt(i);
                i--;
            }
        }
    }

    static int getLoadedBoxesCount() {
        return mLoadedBoxes.count();
    }

    static void clearLoadedBoxes() {
        foreach(BoundingBox *box, mLoadedBoxes) {
            box->clearBoxLoadId();
        }
        mLoadedBoxes.clear();
        foreach(FunctionWaitingForBoxLoad *funcT,
                mFunctionsWaitingForBoxLoad) {
            delete funcT;
        }

        mFunctionsWaitingForBoxLoad.clear();
    }

    virtual void selectAllPoints(Canvas *canvas) {
        Q_UNUSED(canvas);
    }
    virtual void writeBoundingBox(QIODevice *target);
    virtual void readBoundingBox(QIODevice *target);
    virtual void shiftAll(const int &shift);
    virtual QMatrix getRelativeTransformAtRelFrame(const int &relFrame) {
        return mTransformAnimator->getRelativeTransformAtRelFrame(relFrame);
    }
    int prp_getRelFrameShift() const;
    virtual void setupEffects(const int &relFrame,
                              BoundingBoxRenderData *data);

    void addLinkingBox(BoundingBox *box) {
        mLinkingBoxes << box;
    }

    void removeLinkingBox(BoundingBox *box) {
        mLinkingBoxes.removeOne(box);
    }

    const QList<BoundingBox*> &getLinkingBoxes() const {
        return mLinkingBoxes;
    }

    Property *ca_getFirstDescendantWithName(const QString &name);
    EffectAnimators *getEffectsAnimators() {
        return mEffectsAnimators.data();
    }

    void incReasonsNotToApplyUglyTransform() {
        mNReasonsNotToApplyUglyTransform++;
    }

    void decReasonsNotToApplyUglyTransform() {
        mNReasonsNotToApplyUglyTransform--;
    }

    void addChildBox(BoundingBox *box) {
        mChildBoxes.append(box);
    }

    void removeChildBox(BoundingBox *box) {
        mChildBoxes.removeOne(box);
    }

    void clearChildBoxes() {
        foreach(BoundingBox *box, mChildBoxes) {
            box->clearParent();
        }
        mChildBoxes.clear();
    }

    bool isSelected() { return mSelected; }
protected:
    QPointF mSavedTransformPivot;
    bool mSelected = false;
    int mNReasonsNotToApplyUglyTransform = 0;
    QList<BoundingBox*> mChildBoxes;
    QList<BoundingBox*> mLinkingBoxes;
    QList<std::shared_ptr<Updatable> > mSchedulers;
    std::shared_ptr<BoundingBoxRenderData> mCurrentRenderData;

    virtual void updateDrawRenderContainerTransform();

    int mLoadId = -1;

    bool mBlockedSchedule = false;

    DurationRectangle *mDurationRectangle = NULL;
    SingleWidgetAbstraction *mSelectedAbstraction = NULL;
    SingleWidgetAbstraction *mTimelineAbstraction = NULL;

    QPointF mPreviewDrawPos;
    QRectF mRelBoundingRect;
    SkRect mRelBoundingRectSk;

    void updateAfterCombinedTransformationChanged();
    void updateAfterCombinedTransformationChangedAfterFrameChange();

    RenderContainer mDrawRenderContainer;

    bool mUpdateDrawOnParentBox = true;

    bool mRedoUpdate = false;

    SkPath mSkRelBoundingRectPath;

    void setType(const BoundingBoxType &type) { mType = type; }
    BoundingBoxType mType;
    BoxesGroup *mParentGroup = NULL;
    BoundingBox *mParent = NULL;

    QSharedPointer<EffectAnimators> mEffectsAnimators =
            (new EffectAnimators())->ref<EffectAnimators>();

    QSharedPointer<BoxTransformAnimator> mTransformAnimator =
                (new BoxTransformAnimator(this))->ref<BoxTransformAnimator>();

    int mZListIndex = 0;
    bool mPivotAutoAdjust = true;

    QPainter::CompositionMode mCompositionMode =
            QPainter::CompositionMode_SourceOver;
    SkBlendMode mBlendModeSk = SkBlendMode::kSrcOver;

    void getVisibleAbsFrameRange(int *minFrame, int *maxFrame);

    bool mVisible = true;
    bool mLocked = false;

    static QList<BoundingBox*> mLoadedBoxes;
    static QList<FunctionWaitingForBoxLoad*> mFunctionsWaitingForBoxLoad;

signals:
    void nameChanged(QString);
    void scheduledUpdate();
public slots:
    void scheduleUpdate();

    void updateAfterDurationRectangleShifted(const int &dFrame = 0);
    void updateAfterDurationMinFrameChangedBy(const int &by);
    void updateAfterDurationMaxFrameChangedBy(const int &by);
    virtual void updateAfterDurationRectangleRangeChanged() {}

    void prp_updateAfterChangedAbsFrameRange(const int &minFrame,
                                             const int &maxFrame);
    void prp_updateInfluenceRangeAfterChanged();
};


#endif // CHILDPARENT_H
