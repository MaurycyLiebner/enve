#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include "Animators/complexanimator.h"
#include "boundingboxrendercontainer.h"
#include "skiaincludes.h"
#include "renderdatahandler.h"
#include "smartPointers/sharedpointerdefs.h"
#include "colorhelpers.h"

class Canvas;

class MovablePoint;

class NodePoint;
class PathEffect;
class PathAnimator;
class DurationRectangleMovable;
class PaintSettings;
class StrokeSettings;
class PaintSetting;
class EffectAnimators;
class PixmapEffect;
class BoxTransformAnimator;
class BasicTransformAnimator;

class BoxesGroup;
class VectorPathEdge;
class VectorPath;
class DurationRectangle;
struct BoxesGroupRenderData;

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
    TYPE_BONES_BOX
};

struct FunctionWaitingForBoxLoad : public StdSelfRef {
    virtual void boxLoaded(BoundingBox *box) = 0;
    int loadBoxId;

protected:
    FunctionWaitingForBoxLoad(const int &boxIdT);
};

typedef PropertyMimeData<BoundingBox,
    InternalMimeData::BOUNDING_BOX> BoundingBoxMimeData;

class BoundingBox : public ComplexAnimator {
    Q_OBJECT
    friend class SelfRef;
public:
    BoundingBox(const BoundingBoxType &type);
    virtual ~BoundingBox();

    virtual qsptr<BoundingBox> createLink();
    virtual qsptr<BoundingBox> createLinkForLinkGroup();

    void clearEffects();

    virtual void setFont(const QFont &);
    virtual void setSelectedFontSize(const qreal &);
    virtual void setSelectedFontFamilyAndStyle(const QString &,
                                               const QString &);

    virtual QPointF getRelCenterPosition();

    virtual void centerPivotPosition(const bool &saveUndoRedo = false);
    virtual void setPivotPosition(
            const QPointF &pos,
            const bool &saveUndoRedo = false);
    virtual bool isContainedIn(const QRectF &absRect);

    virtual void drawPixmapSk(SkCanvas * const canvas,
                              GrContext* const grContext);
    virtual void drawPixmapSk(SkCanvas *canvas, SkPaint *paint,
                              GrContext* const grContext);
    virtual void drawSelectedSk(SkCanvas *canvas,
                                const CanvasMode &currentCanvasMode,
                                const SkScalar &invScale);


    void applyTransformation(BoxTransformAnimator *transAnimator);
    void applyTransformationInverted(BoxTransformAnimator *transAnimator);

    QPointF getAbsolutePos();

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

    virtual void selectAndAddContainedPointsToList(
            const QRectF &, QList<stdptr<MovablePoint>>&);

    QPointF getPivotAbsPos();
    virtual void select();
    void deselect();
    int getZIndex();
    virtual void drawBoundingRectSk(SkCanvas *canvas,
                                    const SkScalar &invScale);

    virtual void setParentGroup(BoxesGroup *parent);
    void setParentTransform(BasicTransformAnimator *parent);
    void clearParent();

    BoxesGroup *getParentGroup();

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
                                                const qreal &canvasScaleInv);
    virtual void saveTransformPivotAbsPos(const QPointF &absPivot);

    void setName(const QString &name);
    const QString &getName();

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

    virtual void startAllPointsTransform();
    virtual void finishAllPointsTransform();

    virtual void setStrokeCapStyle(const Qt::PenCapStyle &capStyle);
    virtual void setStrokeJoinStyle(const Qt::PenJoinStyle &joinStyle);
    virtual void setStrokeWidth(const qreal &strokeWidth,
                                const bool &finish);

    virtual void startSelectedStrokeWidthTransform();
    virtual void startSelectedStrokeColorTransform();
    virtual void startSelectedFillColorTransform();

    virtual VectorPathEdge *getEdge(const QPointF &absPos,
                                    const qreal &canvasScaleInv);
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
    virtual VectorPath *objectToVectorPathBox();
    virtual VectorPath *strokeToVectorPathBox();

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


    virtual QPointF mapAbsPosToRel(const QPointF &absPos);
    template <class T>
    void addEffect() {
        addEffect(T::template create<T>());
    }

    void addEffect(const qsptr<PixmapEffect> &effect);
    void removeEffect(const qsptr<PixmapEffect> &effect);

    void setBlendModeSk(const SkBlendMode &blendMode);
    virtual const SkBlendMode &getBlendMode();

    virtual void updateAllBoxes(const UpdateReason &reason);
    void selectionChangeTriggered(const bool &shiftPressed);

    bool isAnimated();

    virtual const SkPath &getRelBoundingRectPath();
    virtual QMatrix getRelativeTransformAtCurrentFrame();
    QPointF mapRelPosToAbs(const QPointF &relPos) const;

    QRectF getRelBoundingRect() const;

    virtual QRectF getRelBoundingRectAtRelFrame(const int &relFrame);

    virtual void applyCurrentTransformation();

    virtual Canvas *getParentCanvas();
    virtual void reloadCacheHandler();

    bool SWT_isBoundingBox();

    SingleWidgetAbstraction* SWT_getAbstractionForWidget(
            const UpdateFuncs &updateFuncs,
            const int& visiblePartWidgetId);

    bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                             const bool &parentSatisfies,
                             const bool &parentMainTarget);

    bool SWT_visibleOnlyIfParentDescendant();

    void SWT_addToContextMenu(QMenu *menu);
    bool SWT_handleContextMenuActionSelected(QAction *selectedAction);

    QMimeData *SWT_createMimeData();

    bool isAncestor(BoxesGroup *box) const;
    bool isAncestor(BoundingBox *box) const;
    void removeFromParent_k();
    void removeFromSelection();
    virtual void moveByAbs(const QPointF &trans);
    void copyBoundingBoxDataTo(BoundingBox *targetBox);

    virtual void drawHoveredSk(SkCanvas *canvas,
                               const SkScalar &invScale);

    void drawHoveredPathSk(SkCanvas *canvas,
                           const SkPath &path,
                           const SkScalar &invScale);

    virtual void applyPaintSetting(PaintSetting *setting);

    virtual void setFillColorMode(const ColorMode &colorMode);
    virtual void setStrokeColorMode(const ColorMode &colorMode);
    void switchLocked();

    virtual QMatrix getCombinedTransform() const;

    bool isParticleBox();
    DurationRectangleMovable *anim_getRectangleMovableAtPos(
            const int &relX, const int &minViewedFrame,
            const qreal &pixelsPerFrame);

//    int prp_getParentFrameShift() const;

    void setDurationRectangle(const qsptr<DurationRectangle> &durationRect);

    bool isVisibleAndInVisibleDurationRect();
    void incUsedAsTarget();
    void decUsedAsTarget();
    void ca_childAnimatorIsRecordingChanged();

    virtual void clearAllCache();

    void startPivotTransform();
    void finishPivotTransform();
    bool hasDurationRectangle() const;
    void createDurationRectangle();
    void prp_drawKeys(QPainter *p,
                      const qreal &pixelsPerFrame,
                      const qreal &drawY,
                      const int &startFrame,
                      const int &endFrame,
                      const int &rowHeight,
                      const int &keyRectSize);
    virtual void addPathEffect(const qsptr<PathEffect>&);
    virtual void addFillPathEffect(const qsptr<PathEffect>&);
    virtual void addOutlinePathEffect(const qsptr<PathEffect>&);
    virtual void removePathEffect(const qsptr<PathEffect>&);
    virtual void removeFillPathEffect(const qsptr<PathEffect>&);
    virtual void removeOutlinePathEffect(const qsptr<PathEffect>&);

    virtual void addActionsToMenu(QMenu *);
    virtual bool handleSelectedCanvasAction(QAction *, QWidget*);

    virtual void setupBoundingBoxRenderDataForRelFrameF(
            const qreal &relFrame, BoundingBoxRenderData *data);

    virtual stdsptr<BoundingBoxRenderData> createRenderData();

    virtual qreal getEffectsMarginAtRelFrame(const int &relFrame);
    virtual qreal getEffectsMarginAtRelFrameF(const qreal &relFrame);

    bool prp_differencesBetweenRelFrames(const int &relFrame1, const int &relFrame2);

    bool prp_differencesBetweenRelFramesIncludingInherited(
            const int &relFrame1, const int &relFrame2);
    virtual void renderDataFinished(BoundingBoxRenderData *renderData);
    void updateRelBoundingRectFromRenderData(BoundingBoxRenderData *renderData);

    virtual void updateCurrentPreviewDataFromRenderData(
            BoundingBoxRenderData* renderData);
    virtual bool shouldScheduleUpdate();

//    BoundingBoxRenderData *getCurrentRenderData() {
//        return getCurrentRenderData(anim_mCurrentRelFrame);
//    }
    BoundingBoxRenderData *getCurrentRenderData(const int &relFrame);
//    BoundingBoxRenderData *updateCurrentRenderData() {
//        return updateCurrentRenderData(anim_mCurrentRelFrame);
//    }
    BoundingBoxRenderData *updateCurrentRenderData(const int& relFrame,
                                                      const UpdateReason &reason);
    void nullifyCurrentRenderData(const int& relFrame);
    virtual bool isRelFrameInVisibleDurationRect(const int &relFrame);
    virtual bool isRelFrameFInVisibleDurationRect(const qreal &relFrame);
    bool isRelFrameVisibleAndInVisibleDurationRect(const int &relFrame);
    bool isRelFrameFVisibleAndInVisibleDurationRect(const qreal &relFrame);
    void prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                              int *lastIdentical,
                                              const int &relFrame);
    virtual void getFirstAndLastIdenticalForMotionBlur(int *firstIdentical,
                                               int *lastIdentical,
                                               const int &relFrame,
                                               const bool &takeAncestorsIntoAccount = true);
    virtual void processSchedulers();
    void scheduleTask(const stdsptr<_ScheduledTask> &task);
    virtual void queScheduledTasks();

    const int &getLoadId();

    virtual int setBoxLoadId(const int &loadId);

    virtual void clearBoxLoadId();

    static BoundingBox *getLoadedBoxById(const int &loadId);

    static void addFunctionWaitingForBoxLoad(
            const stdsptr<FunctionWaitingForBoxLoad>& func);

    static void addLoadedBox(BoundingBox *box);

    static int getLoadedBoxesCount();

    static void clearLoadedBoxes();

    virtual void selectAllPoints(Canvas *canvas);
    virtual void writeBoundingBox(QIODevice *target);
    virtual void readBoundingBox(QIODevice *target);

    void writeBoundingBoxDataForLink(QIODevice *target);
    void readBoundingBoxDataForLink(QIODevice *target);

    virtual void shiftAll(const int &shift);
    virtual QMatrix getRelativeTransformAtRelFrame(const int &relFrame);

    virtual QMatrix getRelativeTransformAtRelFrameF(const qreal &relFrame);

    int prp_getRelFrameShift() const;
    virtual void setupEffectsF(const qreal &relFrame,
                               BoundingBoxRenderData* data);

    void addLinkingBox(BoundingBox *box);

    void removeLinkingBox(BoundingBox *box);

    const QList<qptr<BoundingBox>> &getLinkingBoxes() const;

    EffectAnimators *getEffectsAnimators();

    void incReasonsNotToApplyUglyTransform();

    void decReasonsNotToApplyUglyTransform();

    bool isSelected();

    void updateDrawRenderContainerTransform();
    void setPivotAutoAdjust(const bool &bT);

    const BoundingBoxType &getBoxType();

    void startDurationRectPosTransform();
    void finishDurationRectPosTransform();
    void moveDurationRect(const int &dFrame);
    void startMinFramePosTransform();
    void finishMinFramePosTransform();
    void moveMinFrame(const int &dFrame);
    void startMaxFramePosTransform();
    void finishMaxFramePosTransform();
    void moveMaxFrame(const int &dFrame);

    DurationRectangle *getDurationRectangle();

    void requestGlobalPivotUpdateIfSelected();
    void requestGlobalFillStrokeUpdateIfSelected();
protected:
    virtual void getMotionBlurProperties(QList<Property*>& list);

    bool mSelected = false;
    bool mBlockedSchedule = false;
    bool mUpdateDrawOnParentBox = true;
    bool mPivotAutoAdjust = true;
    bool mVisible = true;
    bool mLocked = false;

    int mZListIndex = 0;
    int mNReasonsNotToApplyUglyTransform = 0;
    int mExpiredPixmap = 0;
    int mLoadId = -1;

    BoundingBoxType mType;
    SkBlendMode mBlendModeSk = SkBlendMode::kSrcOver;

    QPointF mSavedTransformPivot;
    QPointF mPreviewDrawPos;

    QRectF mRelBoundingRect;
    SkRect mRelBoundingRectSk;
    SkPath mSkRelBoundingRectPath;

    qptr<BoxesGroup> mParentGroup;
    qptr<BasicTransformAnimator> mParentTransform;

    QList<qptr<BoundingBox>> mChildBoxes;
    QList<qptr<BoundingBox>> mLinkingBoxes;

    RenderDataHandler mCurrentRenderDataHandler;
    stdsptr<RenderContainer> mDrawRenderContainer =
            SPtrCreate(RenderContainer)();

    qsptr<DurationRectangle> mDurationRectangle;

    qsptr<EffectAnimators> mEffectsAnimators;
    qsptr<BoxTransformAnimator> mTransformAnimator;

    QList<stdsptr<_ScheduledTask>> mScheduledTasks;

    static QList<qptr<BoundingBox>> mLoadedBoxes;
    static QList<stdsptr<FunctionWaitingForBoxLoad>> mFunctionsWaitingForBoxLoad;
private:
    void getVisibleAbsFrameRange(int *minFrame, int *maxFrame);
signals:
    void nameChanged(QString);
    void scheduledUpdate();
    void globalPivotInfluenced();
    void fillStrokeSettingsChanged();
public slots:
    void scheduleUpdate(const UpdateReason &reason);
    void scheduleUpdate(const int& relFrame, const UpdateReason &reason);

    void updateAfterDurationRectangleShifted(const int &dFrame = 0);
    void updateAfterDurationMinFrameChangedBy(const int &by);
    void updateAfterDurationMaxFrameChangedBy(const int &by);
    virtual void updateAfterDurationRectangleRangeChanged();

    void prp_updateAfterChangedAbsFrameRange(const int &minFrame,
                                             const int &maxFrame);
    void prp_updateInfluenceRangeAfterChanged();
};


#endif // BOUNDINGBOX_H
