#ifndef CHILDPARENT_H
#define CHILDPARENT_H
#include <QMatrix>
#include "transformable.h"
#include "fillstrokesettings.h"
#include <QSqlQuery>
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

class PathPoint;
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
    TYPE_EXTERNAL_LINK,
    TYPE_PARTICLES
};

class BoxesGroup;

class VectorPathEdge;

class VectorPath;

class DurationRectangle;

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
    SkPoint drawPos;
    SkBlendMode blendMode = SkBlendMode::kSrcOver;

    QSharedPointer<BoundingBox> parentBox;

    virtual void updateRelBoundingRect();
    virtual void drawRenderedImageForParent(SkCanvas *canvas);
    virtual void renderToImage();
    sk_sp<SkImage> renderedImage;

    void processUpdate();

    void beforeUpdate();

    void afterUpdate();

    void schedulerProccessed();

    void addSchedulerNow();

    virtual bool allDataReady() { return true; }

    void dataSet();

protected:
    bool mDataSet = false;
    virtual void drawSk(SkCanvas *canvas) = 0;
};

class BoundingBox :
        public ComplexAnimator,
        public Transformable {
    Q_OBJECT
public:
    BoundingBox(const BoundingBoxType &type);
    virtual ~BoundingBox();

    virtual BoundingBox *createLink();

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
    virtual bool isContainedIn(const QRectF &absRect);

    virtual void drawPixmapSk(SkCanvas *canvas);

    virtual void drawSelectedSk(SkCanvas *canvas,
                                const CanvasMode &currentCanvasMode,
                                const SkScalar &invScale);


    void applyTransformation(BoxTransformAnimator *transAnimator);

    QPointF getAbsolutePos();

    virtual void updateCombinedTransform();
    void updateCombinedTransformAfterFrameChange();

    void moveByRel(const QPointF &trans);

    void startTransform();
    void finishTransform();


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

    virtual void setParent(BoxesGroup *parent);
    BoxesGroup *getParent();

    bool isGroup();
    virtual BoundingBox *getPathAtFromAllAncestors(const QPointF &absPos);

    virtual PaintSettings *getFillSettings();
    virtual StrokeSettings *getStrokeSettings();

    void setPivotAbsPos(const QPointF &absPos,
                        const bool &saveUndoRedo = true,
                        const bool &pivotChanged = true);

    void setPivotRelPos(const QPointF &relPos,
                        const bool &saveUndoRedo = true,
                        const bool &pivotChanged = true);

    void cancelTransform();
    void scale(const qreal &scaleXBy,
               const qreal &scaleYBy);

    virtual int saveToSql(QSqlQuery *query, const int &parentId);

    virtual PathPoint *createNewPointOnLineNear(const QPointF &absPos,
                                                const bool &adjust,
                                                const qreal &canvasScaleInv) {
        Q_UNUSED(absPos);
        Q_UNUSED(adjust);
        Q_UNUSED(canvasScaleInv);
        return NULL;
    }
    bool isVectorPath();
    void saveTransformPivotAbsPos(const QPointF &absPivot);

    void setName(const QString &name);
    QString getName();

    void hide();
    void show();
    bool isVisible();
    void setVisibile(const bool &visible,
                     const bool &saveUndoRedo = true);
    void switchVisible();

    void lock();
    void unlock();
    void setLocked(const bool &bt);
    bool isLocked();
    bool isVisibleAndUnlocked();
    void rotateBy(const qreal &rot);
    void scale(const qreal &scaleBy);

    void rotateRelativeToSavedPivot(const qreal &rot);
    void scaleRelativeToSavedPivot(const qreal &scaleBy);

    virtual void startPosTransform();
    virtual void startRotTransform();
    virtual void startScaleTransform();
    virtual void prp_setAbsFrame(const int &frame);
    virtual QMatrix getCombinedRenderTransform();

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

    virtual VectorPathEdge *getEgde(const QPointF &absPos,
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
    bool isCircle();
    bool isRect();
    bool isText();
    bool isInternalLink();
    bool isExternalLink();
    BoxTransformAnimator *getTransformAnimator();
    void disablePivotAutoAdjust();
    void enablePivotAutoAdjust();

    virtual VectorPath *objectToPath() { return NULL; }
    virtual VectorPath *strokeToPath() { return NULL; }
    virtual void loadFromSql(const int &boundingBoxId);

    void updatePrettyPixmap();

    void saveOldPixmap();

    void saveUglyPaintTransform();
    void drawAsBoundingRectSk(SkCanvas *canvas,
                              const SkPath &path,
                              const SkScalar &invScale,
                              const bool &dashes);

    virtual void setUpdateVars() {}
    void redoUpdate();
    bool shouldRedoUpdate();
    void setRedoUpdateToFalse();


    void updateRelativeTransformTmp();

    virtual QPointF mapAbsPosToRel(const QPointF &absPos);
    void addEffect(PixmapEffect *effect);
    void removeEffect(PixmapEffect *effect);

    void setBlendModeSk(const SkBlendMode &blendMode);
    const SkBlendMode &getBlendMode() {
        return mBlendModeSk;
    }

    virtual QMatrix getCombinedFinalRenderTransform();
    virtual void updateAllBoxes();
    void selectionChangeTriggered(const bool &shiftPressed);

    bool isAnimated() { return prp_isDescendantRecording(); }

    virtual const SkPath &getRelBoundingRectPath();
    virtual QMatrix getRelativeTransform() const;
    QPointF mapRelPosToAbs(const QPointF &relPos) const;

    QRectF getRelBoundingRect() const {
        return mRelBoundingRect;
    }

    virtual QRectF getRelBoundingRectAtRelFrame(const int &relFrame) {
        Q_UNUSED(relFrame);
        return getRelBoundingRect();
    }

    virtual void applyCurrentTransformation() {}

    virtual qreal getEffectsMargin() {
        return mEffectsMargin;
    }

    virtual Canvas *getParentCanvas();


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
    void moveByAbs(const QPointF &trans);
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

    int prp_getFrameShift() const;
    int prp_getParentFrameShift() const;

    void setDurationRectangle(DurationRectangle *durationRect);

    bool isInVisibleDurationRect();
    bool isVisibleAndInVisibleDurationRect();
    void incUsedAsTarget();
    void decUsedAsTarget();
    void ca_childAnimatorIsRecordingChanged();

    int getSqlId() {
        return mSqlId;
    }

    void setSqlId(int id) {
        mSqlId = id;
    }
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
    virtual void addOutlinePathEffect(PathEffect *) {}

    void setCustomFpsEnabled(const bool &bT) {
        mCustomFpsEnabled = bT;
    }

    void enableCustomFps() {
        setCustomFpsEnabled(true);
    }

    void disableCustomFps() {
        setCustomFpsEnabled(false);
    }

    void setCustomFps(const qreal &customFps) {
        mCustomFps = customFps;
    }


    virtual void addActionsToMenu(QMenu *) {}
    virtual bool handleSelectedCanvasAction(QAction *) {
        return false;
    }

    virtual void setupBoundingBoxRenderDataForRelFrame(
            const int &relFrame, BoundingBoxRenderData *data);

    virtual BoundingBoxRenderData *createRenderData() { return NULL; }

    BoundingBoxRenderData *getRenderDataForRelFrame(const int &relFrame) {
        BoundingBoxRenderData *data = createRenderData();
        setupBoundingBoxRenderDataForRelFrame(relFrame, data);
        return data;
    }

    BoundingBoxRenderData *getCurrentRenderData();
    qreal getEffectsMarginAtRelFrame(const int &relFrame);

    bool prp_differencesBetweenRelFrames(const int &relFrame1,
                                         const int &relFrame2);
    virtual void renderDataFinished(BoundingBoxRenderData *renderData);
    void updateRelBoundingRectFromRenderData(BoundingBoxRenderData *renderData);

    virtual void scheduleUpdate();
    virtual void updateCurrentPreviewDataFromRenderData(
            BoundingBoxRenderData *renderData);
    virtual bool shouldScheduleUpdate() {
        if(mParent == NULL) return false;
        if((isVisibleAndInVisibleDurationRect()) ||
           (isInVisibleDurationRect())) {
            return true;
        }
        return false;
    }

    virtual void replaceCurrentFrameCache();
    void updateCurrentRenderData();
    void nullifyCurrentRenderData();
    bool isRelFrameInVisibleDurationRect(const int &relFrame);
    bool isRelFrameVisibleAndInVisibleDurationRect(const int &relFrame);
    void prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                               int *lastIdentical,
                                               const int &relFrame);
    virtual void processSchedulers();
    void addScheduler(Updatable *updatable);
    virtual void addSchedulersToProcess();
protected:
    QList<std::shared_ptr<Updatable> > mSchedulers;
    std::shared_ptr<BoundingBoxRenderData> mCurrentRenderData;
    bool mCustomFpsEnabled = false;
    qreal mCustomFps = 24.;

    void updateDrawRenderContainerTransform();

    bool mRenderCacheChangeNeeded = false;
    bool mReplaceCache = false;

    int mSqlId = 0;

    bool mBlockedSchedule = false;

    DurationRectangle *mDurationRectangle = NULL;
    SingleWidgetAbstraction *mSelectedAbstraction = NULL;
    SingleWidgetAbstraction *mTimelineAbstraction = NULL;

    QPointF mPreviewDrawPos;
    QRectF mRelBoundingRect;
    SkRect mRelBoundingRectSk;

    bool mEffectsMarginUpdateNeeded = false;
    qreal mEffectsMargin = 2.;

    bool mAwaitUpdateScheduled = false;

    virtual void updateAfterCombinedTransformationChanged() {}
    virtual void updateAfterCombinedTransformationChangedAfterFrameChagne() {}


    RenderContainer mDrawRenderContainer;

    bool mUpdateDrawOnParentBox = true;

    bool mRedoUpdate = false;
    bool mLoadingScheduled = false;

    SkPath mSkRelBoundingRectPath;

    int mUsedAsTargetCount = 0;

    bool mScheduledForRemove = false;

    void setType(const BoundingBoxType &type) { mType = type; }
    BoundingBoxType mType;
    QSharedPointer<BoxesGroup> mParent;

    QSharedPointer<EffectAnimators> mEffectsAnimators =
            (new EffectAnimators())->ref<EffectAnimators>();

    QSharedPointer<BoxTransformAnimator> mTransformAnimator =
                (new BoxTransformAnimator(this))->ref<BoxTransformAnimator>();

    int mZListIndex = 0;
    bool mPivotChanged = false;

    QPainter::CompositionMode mCompositionMode =
            QPainter::CompositionMode_SourceOver;
    SkBlendMode mBlendModeSk = SkBlendMode::kSrcOver;

    void getVisibleAbsFrameRange(int *minFrame, int *maxFrame);

    bool mVisible = true;
    bool mLocked = false;
signals:
    void nameChanged(QString);
public slots:
    void updateAfterDurationRectangleShifted(const int &dFrame = 0);
    void updateAfterDurationMinFrameChangedBy(const int &by);
    void updateAfterDurationMaxFrameChangedBy(const int &by);
    virtual void updateAfterDurationRectangleRangeChanged() {}

    void prp_updateAfterChangedAbsFrameRange(const int &minFrame,
                                             const int &maxFrame);
    void prp_updateInfluenceRangeAfterChanged();
};


#endif // CHILDPARENT_H
