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

class BoundingBox : public ComplexAnimator {
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

    virtual void selectAndAddContainedPointsToList(const QRectF &,
                                                   QList<MovablePoint*> *) {}

    QPointF getPivotAbsPos();
    virtual void select();
    void deselect();
    int getZIndex();
    virtual void drawBoundingRectSk(SkCanvas *canvas,
                                    const qreal &invScale);

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
    void addEffect(PixmapEffect *effect);
    void removeEffect(PixmapEffect *effect);

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
    void copyBoundingBoxDataTo(BoundingBox *targetBox);

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

    virtual QMatrix getCombinedTransform() const;

    bool isParticleBox();
    DurationRectangleMovable *anim_getRectangleMovableAtPos(
                                    const qreal &relX,
                                    const int &minViewedFrame,
                                    const qreal &pixelsPerFrame);

//    int prp_getParentFrameShift() const;

    void setDurationRectangle(DurationRectangle *durationRect);

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

    virtual void setupBoundingBoxRenderDataForRelFrameF(
            const qreal &relFrame, const BoundingBoxRenderDataSPtr& data);

    virtual BoundingBoxRenderDataSPtr createRenderData() { return nullptr; }

    virtual qreal getEffectsMarginAtRelFrame(const int &relFrame);
    virtual qreal getEffectsMarginAtRelFrameF(const qreal &relFrame);

    bool prp_differencesBetweenRelFrames(const int &relFrame1, const int &relFrame2);

    bool prp_differencesBetweenRelFramesIncludingInherited(
            const int &relFrame1, const int &relFrame2);
    virtual void renderDataFinished(const BoundingBoxRenderDataSPtr &renderData);
    void updateRelBoundingRectFromRenderData(const BoundingBoxRenderDataSPtr& renderData);

    virtual void updateCurrentPreviewDataFromRenderData(
            const BoundingBoxRenderDataSPtr& renderData);
    virtual bool shouldScheduleUpdate() {
        if(mParentGroup == nullptr) return false;
        if((isVisibleAndInVisibleDurationRect()) ||
           (isRelFrameInVisibleDurationRect(anim_mCurrentRelFrame))) {
            return true;
        }
        return false;
    }

//    BoundingBoxRenderDataSPtr getCurrentRenderData() {
//        return getCurrentRenderData(anim_mCurrentRelFrame);
//    }
    BoundingBoxRenderDataSPtr getCurrentRenderData(const int &relFrame);
//    BoundingBoxRenderDataSPtr updateCurrentRenderData() {
//        return updateCurrentRenderData(anim_mCurrentRelFrame);
//    }
    BoundingBoxRenderDataSPtr updateCurrentRenderData(const int& relFrame,
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
    void addScheduler(_ScheduledExecutor *updatable);
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
        return nullptr;
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
                               const BoundingBoxRenderDataSPtr& data);

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
        return mDurationRectangle;
    }
protected:
    virtual void getMotionBlurProperties(QList<Property*> *list) {
        list->append(mTransformAnimator->getScaleAnimator());
        list->append(mTransformAnimator->getPosAnimator());
        list->append(mTransformAnimator->getPivotAnimator());
        list->append(mTransformAnimator->getRotAnimator());
    }
    int mExpiredPixmap = 0;
    QPointF mSavedTransformPivot;
    bool mSelected = false;
    int mNReasonsNotToApplyUglyTransform = 0;
    QList<BoundingBox*> mChildBoxes;
    QList<BoundingBox*> mLinkingBoxes;
    QList<std::shared_ptr<_ScheduledExecutor> > mSchedulers;
    RenderDataHandler mCurrentRenderDataHandler;


    int mLoadId = -1;

    bool mBlockedSchedule = false;

    DurationRectangle *mDurationRectangle = nullptr;
    SingleWidgetAbstraction *mSelectedAbstraction = nullptr;
    SingleWidgetAbstraction *mTimelineAbstraction = nullptr;

    QPointF mPreviewDrawPos;
    QRectF mRelBoundingRect;
    SkRect mRelBoundingRectSk;

    RenderContainer mDrawRenderContainer;

    bool mUpdateDrawOnParentBox = true;

    SkPath mSkRelBoundingRectPath;

    void setType(const BoundingBoxType &type) { mType = type; }
    BoundingBoxType mType;
    BoxesGroup *mParentGroup = nullptr;
    BasicTransformAnimator *mParentTransform = nullptr;

    QSharedPointer<EffectAnimators> mEffectsAnimators;

    QSharedPointer<BoxTransformAnimator> mTransformAnimator;

    int mZListIndex = 0;
    bool mPivotAutoAdjust = true; // !!! pivot autoadjust disabled

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
