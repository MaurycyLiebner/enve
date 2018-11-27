#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

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

#include "boundingboxrenderdata.h"

#include "renderdatahandler.h"
#include "sharedpointerdefs.h"

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
    TYPE_BONES_BOX
};

class BoxesGroup;
class VectorPathEdge;
class VectorPath;
class DurationRectangle;
class BoxesGroupRenderData;

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

    virtual BoundingBoxQSPtr createLink();
    virtual BoundingBoxQSPtr createLinkForLinkGroup() {
        BoundingBoxQSPtr box = createLink();
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
            const QRectF &, QList<MovablePointPtr>&) {}

    QPointF getPivotAbsPos();
    virtual void select();
    void deselect();
    int getZIndex();
    virtual void drawBoundingRectSk(SkCanvas *canvas,
                                    const SkScalar &invScale);

    virtual void setParentGroup(BoxesGroup *parent);
    virtual void setParent(BasicTransformAnimator *parent);
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
                                                const qreal &canvasScaleInv) {
        Q_UNUSED(absPos);
        Q_UNUSED(adjust);
        Q_UNUSED(canvasScaleInv);
        return nullptr;
    }
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
        return nullptr;
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
    virtual VectorPath *objectToVectorPathBox() { return nullptr; }
    virtual VectorPath *strokeToVectorPathBox() { return nullptr; }

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

    void addEffect(const PixmapEffectQSPtr &effect);
    void removeEffect(const PixmapEffectQSPtr &effect);

    void setBlendModeSk(const SkBlendMode &blendMode);
    virtual const SkBlendMode &getBlendMode() {
        return mBlendModeSk;
    }

    virtual void updateAllBoxes(const UpdateReason &reason);
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

    bool SWT_isBoundingBox() { return true; }

    SingleWidgetAbstraction* SWT_getAbstractionForWidget(
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
    void copyBoundingBoxDataTo(BoundingBox *targetBox);

    virtual void drawHoveredSk(SkCanvas *canvas,
                               const SkScalar &invScale) {
        drawHoveredPathSk(canvas, mSkRelBoundingRectPath, invScale);
    }

    void drawHoveredPathSk(SkCanvas *canvas,
                           const SkPath &path,
                           const SkScalar &invScale);

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

    virtual QMatrix getCombinedTransform() const;

    bool isParticleBox();
    DurationRectangleMovable *anim_getRectangleMovableAtPos(
            const int &relX, const int &minViewedFrame,
            const qreal &pixelsPerFrame);

//    int prp_getParentFrameShift() const;

    void setDurationRectangle(const DurationRectangleQSPtr &durationRect);

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
                      const int &endFrame);
    virtual void addPathEffect(const PathEffectQSPtr&) {}
    virtual void addFillPathEffect(const PathEffectQSPtr&) {}
    virtual void addOutlinePathEffect(const PathEffectQSPtr&) {}
    virtual void removePathEffect(const PathEffectQSPtr&) {}
    virtual void removeFillPathEffect(const PathEffectQSPtr&) {}
    virtual void removeOutlinePathEffect(const PathEffectQSPtr&) {}

    virtual void addActionsToMenu(QMenu *) {}
    virtual bool handleSelectedCanvasAction(QAction *) {
        return false;
    }

    virtual void setupBoundingBoxRenderDataForRelFrameF(
            const qreal &relFrame, BoundingBoxRenderData *data);

    virtual BoundingBoxRenderDataSPtr createRenderData() { return nullptr; }

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
    void addScheduler(const _ScheduledExecutorSPtr &updatable);
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
        foreach(const BoundingBoxQPtr& box, mLoadedBoxes) {
            if(box == nullptr) return nullptr;
            if(box->getLoadId() == loadId) {
                return box;
            }
        }
        return nullptr;
    }

    static void addFunctionWaitingForBoxLoad(
            const FunctionWaitingForBoxLoadSPtr& func) {
        mFunctionsWaitingForBoxLoad << func;
    }

    static void addLoadedBox(BoundingBox *box) {
        mLoadedBoxes << box;
        for(int i = 0; i < mFunctionsWaitingForBoxLoad.count(); i++) {
            FunctionWaitingForBoxLoadSPtr funcT =
                    mFunctionsWaitingForBoxLoad.at(i);
            if(funcT->loadBoxId == box->getLoadId()) {
                funcT->boxLoaded(box);
                mFunctionsWaitingForBoxLoad.removeAt(i);
                i--;
            }
        }
    }

    static int getLoadedBoxesCount() {
        return mLoadedBoxes.count();
    }

    static void clearLoadedBoxes() {
        foreach(const BoundingBoxQPtr& box, mLoadedBoxes) {
            box->clearBoxLoadId();
        }
        mLoadedBoxes.clear();
        mFunctionsWaitingForBoxLoad.clear();
    }

    virtual void selectAllPoints(Canvas *canvas) {
        Q_UNUSED(canvas);
    }
    virtual void writeBoundingBox(QIODevice *target);
    virtual void readBoundingBox(QIODevice *target);

    void writeBoundingBoxDataForLink(QIODevice *target);
    void readBoundingBoxDataForLink(QIODevice *target);

    virtual void shiftAll(const int &shift);
    virtual QMatrix getRelativeTransformAtRelFrame(const int &relFrame) {
        return mTransformAnimator->getRelativeTransformAtRelFrame(relFrame);
    }

    virtual QMatrix getRelativeTransformAtRelFrameF(const qreal &relFrame) {
        return mTransformAnimator->getRelativeTransformAtRelFrameF(relFrame);
    }

    int prp_getRelFrameShift() const;
    virtual void setupEffectsF(const qreal &relFrame,
                               BoundingBoxRenderData* data);

    void addLinkingBox(BoundingBox *box) {
        mLinkingBoxes << box;
    }

    void removeLinkingBox(BoundingBox *box) {
        mLinkingBoxes.removeOne(box);
    }

    const QList<BoundingBoxQPtr> &getLinkingBoxes() const {
        return mLinkingBoxes;
    }

    EffectAnimators *getEffectsAnimators() {
        return mEffectsAnimators.data();
    }

    void incReasonsNotToApplyUglyTransform() {
        mNReasonsNotToApplyUglyTransform++;
    }

    void decReasonsNotToApplyUglyTransform() {
        mNReasonsNotToApplyUglyTransform--;
    }

    bool isSelected() { return mSelected; }

    void updateDrawRenderContainerTransform();
    void setPivotAutoAdjust(const bool &bT) {
        mPivotAutoAdjust = bT;
    }

    const BoundingBoxType &getBoxType() { return mType; }

    void startDurationRectPosTransform();
    void finishDurationRectPosTransform();
    void moveDurationRect(const int &dFrame);
    void startMinFramePosTransform();
    void finishMinFramePosTransform();
    void moveMinFrame(const int &dFrame);
    void startMaxFramePosTransform();
    void finishMaxFramePosTransform();
    void moveMaxFrame(const int &dFrame);

    DurationRectangle *getDurationRectangle() {
        return mDurationRectangle.get();
    }
protected:
    virtual void getMotionBlurProperties(QList<Property*>& list) {
        list.append(mTransformAnimator->getScaleAnimator());
        list.append(mTransformAnimator->getPosAnimator());
        list.append(mTransformAnimator->getPivotAnimator());
        list.append(mTransformAnimator->getRotAnimator());
    }

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

    BoxesGroupQPtr mParentGroup;
    BasicTransformAnimatorQPtr mParentTransform;

    QList<BoundingBoxQPtr> mChildBoxes;
    QList<BoundingBoxQPtr> mLinkingBoxes;

    RenderDataHandler mCurrentRenderDataHandler;
    RenderContainerSPtr mDrawRenderContainer =
            SPtrCreate(RenderContainer)();

    DurationRectangleQSPtr mDurationRectangle;

    QSharedPointer<EffectAnimators> mEffectsAnimators;
    BoxTransformAnimatorQSPtr mTransformAnimator;

    QList<_ScheduledExecutorSPtr> mSchedulers;

    static QList<BoundingBoxQPtr> mLoadedBoxes;
    static QList<FunctionWaitingForBoxLoadSPtr> mFunctionsWaitingForBoxLoad;
private:
    void getVisibleAbsFrameRange(int *minFrame, int *maxFrame);
signals:
    void nameChanged(QString);
    void scheduledUpdate();
public slots:
    void scheduleUpdate(const UpdateReason &reason);
    void scheduleUpdate(const int& relFrame, const UpdateReason &reason);

    void updateAfterDurationRectangleShifted(const int &dFrame = 0);
    void updateAfterDurationMinFrameChangedBy(const int &by);
    void updateAfterDurationMaxFrameChangedBy(const int &by);
    virtual void updateAfterDurationRectangleRangeChanged() {}

    void prp_updateAfterChangedAbsFrameRange(const int &minFrame,
                                             const int &maxFrame);
    void prp_updateInfluenceRangeAfterChanged();
};


#endif // BOUNDINGBOX_H
