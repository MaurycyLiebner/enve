#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include "Animators/complexanimator.h"
#include "boundingboxrendercontainer.h"
#include "skia/skiaincludes.h"
#include "renderdatahandler.h"
#include "smartPointers/sharedpointerdefs.h"
#include "colorhelpers.h"
#include "waitingforboxload.h"
#include "MovablePoints/segment.h"
class Canvas;

class MovablePoint;

class PathEffect;
class DurationRectangleMovable;
class FillSettingsAnimator;
class OutlineSettingsAnimator;
class PaintSettingsApplier;
class EffectAnimators;
class GPUEffectAnimators;
class PixmapEffect;
class BoxTransformAnimator;
class BasicTransformAnimator;

class BoxesGroup;
class SmartVectorPath;
class DurationRectangle;
struct BoxesGroupRenderData;
struct GPURasterEffect;
enum CanvasMode : short;

class SimpleBrushWrapper;

enum BoundingBoxType {
    TYPE_VECTOR_PATH,
    TYPE_CIRCLE,
    TYPE_IMAGE,
    TYPE_RECTANGLE,
    TYPE_TEXT,
    TYPE_GROUP,
    TYPE_CANVAS,
    TYPE_INTERNAL_LINK,
    TYPE_INTERNAL_LINK_GROUP,
    TYPE_INTERNAL_LINK_CANVAS,
    TYPE_EXTERNAL_LINK,
    TYPE_PARTICLES,
    TYPE_VIDEO,
    TYPE_IMAGESQUENCE,
    TYPE_PAINT
};

typedef PropertyMimeData<BoundingBox,
    InternalMimeData::BOUNDING_BOX> BoundingBoxMimeData;

class BoundingBox;
template<typename T> class TypeMenu;
typedef TypeMenu<BoundingBox> BoxTypeMenu;

class BoundingBox : public ComplexAnimator {
    Q_OBJECT
    friend class SelfRef;
protected:
    BoundingBox(const BoundingBoxType &type);
public:
    ~BoundingBox();

    static BoundingBox *sGetBoxByDocumentId(const int &documentId);

    static void sClearWriteBoxes();

    static void sAddReadBox(BoundingBox * const box);
    static BoundingBox *sGetBoxByReadId(const int &readId);
    static void sClearReadBoxes();
    static void sAddWaitingForBoxLoad(const WaitingForBoxLoad& func);
private:
    static int sNextDocumentId;
    static QList<BoundingBox*> sDocumentBoxes;

    static QList<BoundingBox*> sReadBoxes;
    static QList<WaitingForBoxLoad> sFunctionsWaitingForBoxRead;

    static int sNextWriteId;
    static QList<BoundingBox*> sBoxesWithWriteIds;
public slots:
    virtual void updateAfterDurationRectangleRangeChanged();

    void prp_updateAfterChangedAbsFrameRange(const FrameRange &range);
    void prp_updateInfluenceRangeAfterChanged();
protected:
    virtual void getMotionBlurProperties(QList<Property*> &list) const;
public:
    virtual void setParentGroup(BoxesGroup * const parent);

    virtual qsptr<BoundingBox> createLink();
    virtual qsptr<BoundingBox> createLinkForLinkGroup();

    virtual SmartVectorPath *objectToVectorPathBox();
    virtual SmartVectorPath *strokeToVectorPathBox();

    virtual void moveByRel(const QPointF &trans);
    virtual void moveByAbs(const QPointF &trans);
    virtual void rotateBy(const qreal &rot);
    virtual void scale(const qreal &scaleBy);
    virtual void scale(const qreal &scaleXBy, const qreal &scaleYBy);
    virtual void saveTransformPivotAbsPos(const QPointF &absPivot);

    virtual void startPosTransform();
    virtual void startRotTransform();
    virtual void startScaleTransform();

    virtual void startTransform();
    virtual void finishTransform();
    virtual void cancelTransform();

    virtual void startAllPointsTransform();
    virtual void finishAllPointsTransform();

    virtual void centerPivotPosition();
    virtual QPointF getRelCenterPosition();

    virtual void selectAndAddContainedPointsToList(
            const QRectF &, QList<stdptr<MovablePoint>>&);
    virtual bool relPointInsidePath(const QPointF &relPos) const;

    virtual void setFont(const QFont &);
    virtual void setSelectedFontSize(const qreal &);
    virtual void setSelectedFontFamilyAndStyle(const QString &,
                                               const QString &);

    virtual void drawPixmapSk(SkCanvas * const canvas,
                              GrContext* const grContext);
    virtual void drawPixmapSk(SkCanvas * const canvas,
                              SkPaint * const paint,
                              GrContext* const grContext);
    void drawCanvasControls(SkCanvas * const canvas,
                            const CanvasMode &currentCanvasMode,
                            const SkScalar &invScale);
    virtual void drawHoveredSk(SkCanvas *canvas,
                               const SkScalar &invScale);
    virtual void drawBoundingRect(SkCanvas * const canvas,
                                  const SkScalar &invScale);

    virtual const SkPath &getRelBoundingRectPath();
    virtual QRectF getRelBoundingRect(const qreal &relFrame);

    virtual MovablePoint *createNewPointOnLineNear(const QPointF &absPos,
                                                   const bool &adjust,
                                                   const qreal &canvasScaleInv);

    virtual BoundingBox *getBoxAtFromAllDescendents(const QPointF &absPos);

    virtual NormalSegment getNormalSegment(const QPointF &absPos,
                                           const qreal &canvasScaleInv) {
        Q_UNUSED(absPos);
        Q_UNUSED(canvasScaleInv);
        return NormalSegment();
    }
    virtual MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                                     const CanvasMode &currentCanvasMode,
                                     const qreal &canvasScaleInv);

    virtual FillSettingsAnimator *getFillSettings() const;
    virtual OutlineSettingsAnimator *getStrokeSettings() const;

    virtual void setStrokeCapStyle(const Qt::PenCapStyle &capStyle);
    virtual void setStrokeJoinStyle(const Qt::PenJoinStyle &joinStyle);
    virtual void setStrokeWidth(const qreal &strokeWidth);

    virtual void setStrokeBrush(SimpleBrushWrapper * const brush) {
        Q_UNUSED(brush);
    }
    virtual void setStrokeBrushWidthCurve(
            const qCubicSegment1D& curve) {
        Q_UNUSED(curve);
    }
    virtual void setStrokeBrushTimeCurve(
            const qCubicSegment1D& curve) {
        Q_UNUSED(curve);
    }
    virtual void setStrokeBrushPressureCurve(
            const qCubicSegment1D& curve) {
        Q_UNUSED(curve);
    }
    virtual void setStrokeBrushSpacingCurve(
            const qCubicSegment1D& curve) {
        Q_UNUSED(curve);
    }

    virtual void startSelectedStrokeWidthTransform();
    virtual void startSelectedStrokeColorTransform();
    virtual void startSelectedFillColorTransform();

    virtual void updateAllBoxes(const UpdateReason &reason);

    virtual QMatrix getRelativeTransformAtCurrentFrame();
    virtual QMatrix getRelativeTransformAtRelFrameF(const qreal &relFrame);
    virtual QMatrix getTotalTransform() const;
    virtual QPointF mapAbsPosToRel(const QPointF &absPos);

    virtual Canvas *getParentCanvas();
    virtual void reloadCacheHandler();

    virtual void applyPaintSetting(const PaintSettingsApplier &setting);

    virtual void setFillColorMode(const ColorMode &colorMode);
    virtual void setStrokeColorMode(const ColorMode &colorMode);

    virtual void addPathEffect(const qsptr<PathEffect>&);
    virtual void addFillPathEffect(const qsptr<PathEffect>&);
    virtual void addOutlinePathEffect(const qsptr<PathEffect>&);
    virtual void removePathEffect(const qsptr<PathEffect>&);
    virtual void removeFillPathEffect(const qsptr<PathEffect>&);
    virtual void removeOutlinePathEffect(const qsptr<PathEffect>&);

    virtual void addActionsToMenu(BoxTypeMenu * const menu);

    virtual stdsptr<BoundingBoxRenderData> createRenderData();
    virtual void setupRenderData(const qreal &relFrame,
                                 BoundingBoxRenderData * const data);
    virtual void renderDataFinished(BoundingBoxRenderData *renderData);
    virtual void updateCurrentPreviewDataFromRenderData(
            BoundingBoxRenderData* renderData);

    virtual qreal getEffectsMarginAtRelFrame(const int &relFrame);
    virtual qreal getEffectsMarginAtRelFrameF(const qreal &relFrame);

    virtual bool isRelFrameInVisibleDurationRect(const int &relFrame) const;
    virtual bool isRelFrameFInVisibleDurationRect(const qreal &relFrame) const;
    virtual void shiftAll(const int &shift);

    virtual FrameRange getFirstAndLastIdenticalForMotionBlur(
            const int &relFrame, const bool &takeAncestorsIntoAccount = true);

    virtual bool shouldScheduleUpdate();
    virtual void scheduleWaitingTasks();
    virtual void queScheduledTasks();

    virtual void writeBoundingBox(QIODevice *target);
    virtual void readBoundingBox(QIODevice *target);

    virtual void setupEffectsF(const qreal &relFrame,
                               BoundingBoxRenderData * const data);
    virtual void setupGPUEffectsF(const qreal &relFrame,
                                  BoundingBoxRenderData * const data);
    virtual const SkBlendMode &getBlendMode();

    bool SWT_isBoundingBox() const;
    bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                             const bool &parentSatisfies,
                             const bool &parentMainTarget) const;
    bool SWT_visibleOnlyIfParentDescendant() const;
    QMimeData *SWT_createMimeData();

    FrameRange prp_getIdenticalRelFrameRange(const int &relFrame) const;
    int prp_getRelFrameShift() const;

    void anim_setAbsFrame(const int &frame);
    DurationRectangleMovable *anim_getRectangleMovableAtPos(
            const int &relX, const int &minViewedFrame,
            const qreal &pixelsPerFrame);
    void drawTimelineControls(QPainter * const p,
                              const qreal &pixelsPerFrame,
                              const FrameRange &absFrameRange,
                              const int &rowHeight);

    void ca_childAnimatorIsRecordingChanged();

    void selectAllCanvasPts(QList<stdptr<MovablePoint>> &selection);

    int getDocumentId() const { return mDocumentId; }

    int assignWriteId();
    void clearWriteId();
    int getWriteId() const;

    int getReadId() const;
    void clearReadId();

    void clearParent();
    BoxesGroup *getParentGroup() const;
    void setParentTransform(BasicTransformAnimator *parent);
    bool isParentLinkBox();

    bool isContainedIn(const QRectF &absRect) const;

    void applyTransformation(BoxTransformAnimator *transAnimator);
    void applyTransformationInverted(BoxTransformAnimator *transAnimator);

    QPointF getPivotAbsPos();
    QPointF getAbsolutePos() const;
    bool absPointInsidePath(const QPointF &absPos);

    void moveUp();
    void moveDown();
    void bringToFront();
    void bringToEnd();
    void setZListIndex(const int &z);
    int getZIndex() const;

    void setPivotAbsPos(const QPointF &absPos);
    void setPivotRelPos(const QPointF &relPos);

    void setName(const QString &name);
    const QString &getName() const;
    void select();
    void deselect();
    bool isSelected() const;
    void removeFromSelection();
    void selectionChangeTriggered(const bool &shiftPressed);
    void hide();
    void show();
    bool isVisible() const;
    void setVisibile(const bool &visible);
    void switchVisible();
    void lock();
    void unlock();
    void setLocked(const bool &bt);
    void switchLocked();
    bool isLocked() const;
    bool isVisibleAndUnlocked() const;
    bool isAnimated() const;

    void rotateRelativeToSavedPivot(const qreal &rot);
    void scaleRelativeToSavedPivot(const qreal &scaleBy);
    void setAbsolutePos(const QPointF &pos);
    void setRelativePos(const QPointF &relPos);

    void scaleRelativeToSavedPivot(const qreal &scaleXBy,
                                   const qreal &scaleYBy);
    void startPivotTransform();
    void finishPivotTransform();
    void resetScale();
    void resetTranslation();
    void resetRotation();

    BoxTransformAnimator *getTransformAnimator();
    QRectF getRelBoundingRect() const;
    void drawHoveredPathSk(SkCanvas *canvas,
                           const SkPath &path,
                           const SkScalar &invScale);
    void drawOutlineOverlay(SkCanvas * const canvas,
                              const SkPath &path,
                              const SkScalar &invScale,
                              const bool &dashes);

    template <class T>
    void addEffect() {
        addEffect(SPtrCreateTemplated(T)());
    }

    void addEffect(const qsptr<PixmapEffect> &effect);
    void removeEffect(const qsptr<PixmapEffect> &effect);
    void clearEffects();

    void addGPUEffect(const qsptr<GPURasterEffect> &rasterEffect);
    void removeGPUEffect(const qsptr<GPURasterEffect> &effect);

    void setBlendModeSk(const SkBlendMode &blendMode);

    QPointF mapRelPosToAbs(const QPointF &relPos) const;

    bool isAncestor(const BoundingBox * const box) const;
    void removeFromParent_k();

    void copyBoundingBoxDataTo(BoundingBox * const targetBox);


//    int prp_getParentFrameShift() const;

    void setDurationRectangle(const qsptr<DurationRectangle> &durationRect);
    bool hasDurationRectangle() const;
    void createDurationRectangle();
    bool isVisibleAndInVisibleDurationRect() const;
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
    bool isRelFrameVisibleAndInVisibleDurationRect(const int &relFrame) const;
    bool isRelFrameFVisibleAndInVisibleDurationRect(const qreal &relFrame) const;
    bool diffsIncludingInherited(
            const int &relFrame1, const int &relFrame2);

    BoundingBoxRenderData *getCurrentRenderData(const int &relFrame);
    BoundingBoxRenderData *updateCurrentRenderData(const int& relFrame,
                                                   const UpdateReason &reason);
    void nullifyCurrentRenderData(const int& relFrame);
    void updateDrawRenderContainerTransform();

    void scheduleTask(const stdsptr<_ScheduledTask> &task);

    void addLinkingBox(BoundingBox *box);
    void removeLinkingBox(BoundingBox *box);
    const QList<qptr<BoundingBox>> &getLinkingBoxes() const;

    EffectAnimators *getEffectsAnimators();

    void incReasonsNotToApplyUglyTransform();
    void decReasonsNotToApplyUglyTransform();

    const BoundingBoxType &getBoxType() const;

    void requestGlobalPivotUpdateIfSelected();
    void requestGlobalFillStrokeUpdateIfSelected();
    QMatrix getTotalTransformAtRelFrame(const qreal &relFrame);
    QMatrix getParentTotalTransformAtRelFrame(const qreal &relFrame);
protected:
    void updateCanvasProps() {
        mCanvasProps.clear();
        ca_execOnDescendants([this](Property * prop) {
            if(prop->drawsOnCanvas()) mCanvasProps.append(prop);
        });
    }

    bool mSelected = false;
    bool mInVisibleRange = true;
    bool mVisible = true;
    bool mLocked = false;
    uint mStateId = 0;

    int mZListIndex = 0;
    int mNReasonsNotToApplyUglyTransform = 0;
    int mReadId = -1;
    int mWriteId = -1;
    const int mDocumentId;

    BoundingBoxType mType;
    SkBlendMode mBlendModeSk = SkBlendMode::kSrcOver;

    QPointF mSavedTransformPivot;

    QRectF mRelBoundingRect;
    SkRect mRelBoundingRectSk;
    SkPath mSkRelBoundingRectPath;

    qptr<BoxesGroup> mParentGroup;
    qptr<BasicTransformAnimator> mParentTransform;

    QList<qptr<BoundingBox>> mChildBoxes;
    QList<qptr<BoundingBox>> mLinkingBoxes;

    RenderDataHandler mCurrentRenderDataHandler;
    RenderContainer mDrawRenderContainer;

    qsptr<DurationRectangle> mDurationRectangle;
    const qsptr<BoxTransformAnimator> mTransformAnimator;

    const qsptr<EffectAnimators> mEffectsAnimators;
    const qsptr<GPUEffectAnimators> mGPUEffectsAnimators;

    QList<stdsptr<_ScheduledTask>> mScheduledTasks;
    QList<qptr<Property>> mCanvasProps;
private:
    void updateRelBoundingRectFromRenderData(
            BoundingBoxRenderData * const renderData);
    FrameRange getVisibleAbsFrameRange() const;
    void cancelWaitingTasks();
    void afterTotalTransformChanged(const UpdateReason &reason);
signals:
    void nameChanged(QString);
    void scheduledUpdate();
    void globalPivotInfluenced();
    void fillStrokeSettingsChanged();
public slots:
    void scheduleUpdate(const UpdateReason &reason);
    void scheduleUpdate(const int& relFrame, const UpdateReason &reason);

    void updateAfterDurationRectangleShifted(const int &dFrame);
    void updateAfterDurationMinFrameChangedBy(const int &by);
    void updateAfterDurationMaxFrameChangedBy(const int &by);
};


#endif // BOUNDINGBOX_H
