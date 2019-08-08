#ifndef BOUNDINGBOX_H
#define BOUNDINGBOX_H

#include "Animators/staticcomplexanimator.h"
#include "boxrendercontainer.h"
#include "skia/skiaincludes.h"
#include "renderdatahandler.h"
#include "smartPointers/ememory.h"
#include "colorhelpers.h"
#include "waitingforboxload.h"
#include "MovablePoints/segment.h"
class Canvas;

class QrealAction;
class MovablePoint;

class PathEffect;
class DurationRectangleMovable;
class FillSettingsAnimator;
class OutlineSettingsAnimator;
class PaintSettingsApplier;
class RasterEffectAnimators;
class ShaderEffectProgram;
class BoxTransformAnimator;
class BasicTransformAnimator;

class ContainerBox;
class SmartVectorPath;
class DurationRectangle;
struct ContainerBoxRenderData;
class ShaderEffect;
class RasterEffect;
enum CanvasMode : short;

class SimpleBrushWrapper;

enum eBoxType {
    TYPE_VECTOR_PATH,
    TYPE_CIRCLE,
    TYPE_IMAGE,
    TYPE_RECTANGLE,
    TYPE_TEXT,
    TYPE_LAYER,
    TYPE_CANVAS,
    TYPE_INTERNAL_LINK,
    TYPE_INTERNAL_LINK_GROUP,
    TYPE_INTERNAL_LINK_CANVAS,
    TYPE_EXTERNAL_LINK,
    TYPE_PARTICLES,
    TYPE_VIDEO,
    TYPE_IMAGESQUENCE,
    TYPE_PAINT,
    TYPE_GROUP,
    TYPE_CUSTOM
};

class BoundingBox;
template<typename T> class TypeMenu;
typedef TypeMenu<Property> PropertyMenu;

class BoundingBox : public StaticComplexAnimator {
    Q_OBJECT
    e_OBJECT
protected:
    BoundingBox(const eBoxType type);
public:
    ~BoundingBox();

    virtual stdsptr<BoxRenderData> createRenderData() = 0;

    static BoundingBox *sGetBoxByDocumentId(const int documentId);

    static void sClearWriteBoxes();

    static void sAddReadBox(BoundingBox * const box);
    static BoundingBox *sGetBoxByReadId(const int readId);
    static void sClearReadBoxes();
    static void sAddWaitingForBoxLoad(const WaitingForBoxLoad& func);

    static SkFilterQuality sDisplayFiltering;
private:
    static int sNextDocumentId;
    static QList<BoundingBox*> sDocumentBoxes;

    static QList<BoundingBox*> sReadBoxes;
    static QList<WaitingForBoxLoad> sFunctionsWaitingForBoxRead;

    static int sNextWriteId;
    static QList<BoundingBox*> sBoxesWithWriteIds;
protected:
    virtual void getMotionBlurProperties(QList<Property*> &list) const;
public:
    virtual void updateAfterDurationRectangleRangeChanged();
    virtual void setParentGroup(ContainerBox * const parent);

    virtual qsptr<BoundingBox> createLink();
    virtual qsptr<BoundingBox> createLinkForLinkGroup();

    virtual SmartVectorPath *objectToVectorPathBox();
    virtual SmartVectorPath *strokeToVectorPathBox();

    void moveByRel(const QPointF &trans);
    void moveByAbs(const QPointF &trans);
    void rotateBy(const qreal rot);
    void scale(const qreal scaleBy);
    void scale(const qreal scaleXBy, const qreal scaleYBy);
    void saveTransformPivotAbsPos(const QPointF &absPivot);

    void startPosTransform();
    void startRotTransform();
    void startScaleTransform();

    void startTransform();
    void finishTransform();
    void cancelTransform();

    virtual void centerPivotPosition();
    virtual QPointF getRelCenterPosition();

    virtual void selectAndAddContainedPointsToList(
            const QRectF &absRect, QList<MovablePoint*> &selection,
            const CanvasMode mode);

    virtual bool relPointInsidePath(const QPointF &relPos) const;

    virtual void setFont(const QFont &font) {
        Q_UNUSED(font);
    }

    virtual void setSelectedFontSize(const qreal fontSize) {
        Q_UNUSED(fontSize);
    }

    virtual void setSelectedFontFamilyAndStyle(const QString &family,
                                               const QString &style) {
        Q_UNUSED(family);
        Q_UNUSED(style);
    }

    virtual void drawPixmapSk(SkCanvas * const canvas);
    virtual void drawPixmapSk(SkCanvas * const canvas,
                              SkPaint * const paint);
    virtual void drawHoveredSk(SkCanvas *canvas,
                               const float invScale);

    virtual const SkPath &getRelBoundingRectPath();

    virtual BoundingBox *getBoxAtFromAllDescendents(const QPointF &absPos);

    virtual FillSettingsAnimator *getFillSettings() const;
    virtual OutlineSettingsAnimator *getStrokeSettings() const;

    virtual void setStrokeCapStyle(const SkPaint::Cap capStyle);
    virtual void setStrokeJoinStyle(const SkPaint::Join joinStyle);

    virtual void strokeWidthAction(const QrealAction& action)
    { Q_UNUSED(action); }

    virtual void startSelectedStrokeColorTransform();
    virtual void startSelectedFillColorTransform();

    virtual void updateAllBoxes(const UpdateReason reason);

    virtual QMatrix getRelativeTransformAtCurrentFrame();
    virtual QMatrix getRelativeTransformAtFrame(const qreal relFrame);
    virtual QMatrix getTotalTransformAtFrame(const qreal relFrame);
    virtual QPointF mapAbsPosToRel(const QPointF &absPos);

    virtual void applyPaintSetting(const PaintSettingsApplier &setting);

    virtual void addPathEffect(const qsptr<PathEffect>&);
    virtual void addFillPathEffect(const qsptr<PathEffect>&);
    virtual void addOutlineBasePathEffect(const qsptr<PathEffect>&);
    virtual void addOutlinePathEffect(const qsptr<PathEffect>&);
    virtual void removePathEffect(const qsptr<PathEffect>&);
    virtual void removeFillPathEffect(const qsptr<PathEffect>&);
    virtual void removeOutlinePathEffect(const qsptr<PathEffect>&);

    void setupTreeViewMenu(PropertyMenu * const menu) final;
    virtual void setupCanvasMenu(PropertyMenu * const menu);

    virtual void setupRenderData(const qreal relFrame,
                                 BoxRenderData * const data);
    virtual void renderDataFinished(BoxRenderData *renderData);
    virtual void updateCurrentPreviewDataFromRenderData(
            BoxRenderData* renderData);

    virtual bool isFrameInDurationRect(const int relFrame) const;
    virtual bool isFrameFInDurationRect(const qreal relFrame) const;
    virtual void shiftAll(const int shift);

    virtual FrameRange getFirstAndLastIdenticalForMotionBlur(
            const int relFrame, const bool takeAncestorsIntoAccount = true);

    virtual bool shouldScheduleUpdate() { return true; }
    virtual void queScheduledTasks();

    virtual void writeIdentifier(QIODevice * const dst) const;

    virtual void writeBoundingBox(QIODevice * const dst);
    virtual void readBoundingBox(QIODevice * const src);

    virtual void setupRasterEffectsF(const qreal relFrame,
                                  BoxRenderData * const data);
    virtual const SkBlendMode &getBlendMode();

    bool SWT_isBoundingBox() const;
    bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                             const bool parentSatisfies,
                             const bool parentMainTarget) const;
    bool SWT_visibleOnlyIfParentDescendant() const;

    bool SWT_dropSupport(const QMimeData* const data);
    bool SWT_drop(const QMimeData* const data);
    QMimeData *SWT_createMimeData();

    void prp_afterChangedAbsRange(const FrameRange &range);
    void drawAllCanvasControls(SkCanvas * const canvas,
                               const CanvasMode mode,
                               const float invScale);

    FrameRange prp_relInfluenceRange() const;
    FrameRange prp_getIdenticalRelRange(const int relFrame) const;
    int prp_getRelFrameShift() const;

    void anim_setAbsFrame(const int frame);
    DurationRectangleMovable *anim_getTimelineMovable(
            const int relX, const int minViewedFrame,
            const qreal pixelsPerFrame);
    void drawTimelineControls(QPainter * const p,
                              const qreal pixelsPerFrame,
                              const FrameRange &absFrameRange,
                              const int rowHeight);

    void ca_childAnimatorIsRecordingChanged();

    Canvas* getParentScene() const { return mParentScene; }

    QMatrix getTotalTransform() const;

    MovablePoint *getPointAtAbsPos(const QPointF &absPos,
                                   const CanvasMode mode,
                                   const qreal invScale) const;
    NormalSegment getNormalSegment(const QPointF &absPos,
                                   const qreal invScale) const;
    void drawBoundingRect(SkCanvas * const canvas,
                          const float invScale);

    void selectAllCanvasPts(QList<MovablePoint *> &selection,
                            const CanvasMode mode);

    int getDocumentId() const { return mDocumentId; }

    int assignWriteId();
    void clearWriteId();
    int getWriteId() const;

    int getReadId() const;
    void clearReadId();

    void clearParent();
    ContainerBox *getParentGroup() const;
    void setParentTransform(BasicTransformAnimator *parent);
    bool isParentLinkBox();

    bool isContainedIn(const QRectF &absRect) const;

    QPointF getPivotAbsPos();
    QPointF getAbsolutePos() const;
    bool absPointInsidePath(const QPointF &absPos);

    void moveUp();
    void moveDown();
    void bringToFront();
    void bringToEnd();
    void setZListIndex(const int z);
    int getZIndex() const;

    void setPivotAbsPos(const QPointF &absPos);
    void setPivotRelPos(const QPointF &relPos);

    void setSelected(const bool select);
    void select();
    void deselect();
    bool isSelected() const;
    void selectionChangeTriggered(const bool shiftPressed);
    void hide();
    void show();
    bool isVisible() const;
    void setVisibile(const bool visible);
    void switchVisible();
    void lock();
    void unlock();
    void setLocked(const bool bt);
    void switchLocked();
    bool isLocked() const;
    bool isVisibleAndUnlocked() const;
    bool isAnimated() const;

    void rotateRelativeToSavedPivot(const qreal rot);
    void scaleRelativeToSavedPivot(const qreal scaleBy);
    void setAbsolutePos(const QPointF &pos);
    void setRelativePos(const QPointF &relPos);

    void scaleRelativeToSavedPivot(const qreal scaleXBy,
                                   const qreal scaleYBy);
    void startPivotTransform();
    void finishPivotTransform();
    void resetScale();
    void resetTranslation();
    void resetRotation();

    BasicTransformAnimator *getTransformAnimator() const;
    BoxTransformAnimator *getBoxTransformAnimator() const;
    QRectF getRelBoundingRect() const;
    void drawHoveredPathSk(SkCanvas *canvas,
                           const SkPath &path,
                           const float invScale);

    void setRasterEffectsEnabled(const bool enable);
    bool getRasterEffectsEnabled() const;

    template <class T>
    void addEffect() {
        addEffect(enve::make_shared<T>());
    }

    void clearRasterEffects();

    void addRasterEffect(const qsptr<RasterEffect> &rasterEffect);
    void removeRasterEffect(const qsptr<RasterEffect> &effect);

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
    void moveDurationRect(const int dFrame);
    void startMinFramePosTransform();
    void finishMinFramePosTransform();
    void moveMinFrame(const int dFrame);
    void startMaxFramePosTransform();
    void finishMaxFramePosTransform();
    void moveMaxFrame(const int dFrame);
    DurationRectangle *getDurationRectangle();
    bool isVisibleAndInDurationRect(const int relFrame) const;
    bool isFrameFVisibleAndInDurationRect(const qreal relFrame) const;
    bool diffsIncludingInherited(const int relFrame1, const int relFrame2) const;
    bool diffsIncludingInherited(const qreal relFrame1, const qreal relFrame2) const;

    bool hasCurrentRenderData(const qreal relFrame) const;
    stdsptr<BoxRenderData> getCurrentRenderData(const qreal relFrame) const;
    BoxRenderData *updateCurrentRenderData(const qreal relFrame,
                                           const UpdateReason reason);

    void updateDrawRenderContainerTransform();

    void scheduleTask(const stdsptr<BoxRenderData> &task);

    void addLinkingBox(BoundingBox *box);
    void removeLinkingBox(BoundingBox *box);
    const QList<BoundingBox*> &getLinkingBoxes() const;


    void incReasonsNotToApplyUglyTransform();
    void decReasonsNotToApplyUglyTransform();

    eBoxType getBoxType() const;

    void requestGlobalPivotUpdateIfSelected();
    void requestGlobalFillStrokeUpdateIfSelected();

    void planScheduleUpdate(const UpdateReason reason);

    void updateAfterDurationRectangleShifted(const int dFrame);
    void updateAfterDurationMinFrameChangedBy(const int by);
    void updateAfterDurationMaxFrameChangedBy(const int by);

    void planCenterPivotPosition();

    virtual void updateIfUsesProgram(
            const ShaderEffectProgram * const program) const;
protected:
    void setRelBoundingRect(const QRectF& relRect);

    void updateCanvasProps() {
        mCanvasProps.clear();
        ca_execOnDescendants([this](Property * prop) {
            if(prop->drawsOnCanvas()) mCanvasProps.append(prop);
        });
        if(drawsOnCanvas()) mCanvasProps.append(this);
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

    eBoxType mType;
    SkBlendMode mBlendModeSk = SkBlendMode::kSrcOver;

    QPointF mSavedTransformPivot;

    QRectF mRelRect;
    SkRect mRelRectSk;
    SkPath mSkRelBoundingRectPath;

    Canvas* mParentScene = nullptr;
    qptr<ContainerBox> mParentGroup;
    qptr<BasicTransformAnimator> mParentTransform;

    QList<BoundingBox*> mLinkingBoxes;

    RenderDataHandler mCurrentRenderDataHandler;
    RenderContainer mDrawRenderContainer;

    bool mDurationRectangleLocked = false;
    qsptr<DurationRectangle> mDurationRectangle;
    const qsptr<BoxTransformAnimator> mTransformAnimator;

    const qsptr<RasterEffectAnimators> mRasterEffectsAnimators;

    bool mCenterPivotPlanned = false;
    bool mSchedulePlanned = false;
    UpdateReason mPlannedReason;
    QList<stdsptr<BoxRenderData>> mScheduledTasks;
    QList<qptr<Property>> mCanvasProps;
private:
    void scheduleUpdate();
    FrameRange getVisibleAbsFrameRange() const;
    void cancelWaitingTasks();
    void afterTotalTransformChanged(const UpdateReason reason);
signals:
    void globalPivotInfluenced();
    void fillStrokeSettingsChanged();
    void selectionChanged(bool);
    void visibilityChanged(bool);
    void parentChanged(ContainerBox*);
    void ancestorChanged();
};


#endif // BOUNDINGBOX_H
