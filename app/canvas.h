#ifndef CANVAS_H
#define CANVAS_H

#include "Boxes/boxesgroup.h"
#include "colorhelpers.h"
#include <QThread>
#include "Boxes/rendercachehandler.h"
#include "skiaincludes.h"
#include "GUI/valueinput.h"
#include "GUI/canvaswindow.h"
#include "Animators/coloranimator.h"

class TextBox;
class Circle;
class ParticleBox;
class Rectangle;
class PathPivot;
class SoundComposition;
class SkCanvas;
class ImageSequenceBox;
class Brush;
class NodePoint;
class UndoRedoStack;
class ExternalLinkBox;
class Bone;

#define getAtIndexOrGiveNull(index, list) (( (index) >= (list).count() || (index) < 0 ) ? nullptr : (list).at( (index) ))

#define Q_FOREACHInverted(item, list) item = getAtIndexOrGiveNull((list).count() - 1, (list)); \
    for(int i = (list).count() - 1; i >= 0; i--, item = getAtIndexOrGiveNull(i, (list)) )
#define Q_FOREACHInverted2(item, list) for(int i = (list).count() - 1; i >= 0; i--) { \
            item = getAtIndexOrGiveNull(i, (list));

enum CtrlsMode : short;

enum CanvasMode : short {
    MOVE_PATH,
    MOVE_POINT,
    ADD_POINT,
    PICK_PAINT_SETTINGS,
    ADD_CIRCLE,
    ADD_RECTANGLE,
    ADD_TEXT,
    ADD_PARTICLE_BOX,
    ADD_PARTICLE_EMITTER,
    ADD_PAINT_BOX,
    PAINT_MODE,
    ADD_BONE
};


extern bool zLessThan(const qptr<BoundingBox> &box1,
                      const qptr<BoundingBox> &box2);

struct CanvasRenderData : public BoxesGroupRenderData {
    CanvasRenderData(BoundingBox *parentBoxT);
    void renderToImage();
    qreal canvasWidth;
    qreal canvasHeight;
    SkColor bgColor;
protected:
    void drawSk(SkCanvas *canvas);

    void updateRelBoundingRect();
};

extern bool boxesZSort(const qptr<BoundingBox> &box1,
                       const qptr<BoundingBox> &box2);

class Canvas : public BoxesGroup {
    Q_OBJECT
    friend class SelfRef;
public:
    explicit Canvas(CanvasWindow *canvasWidget,
                    int canvasWidth = 1920,
                    int canvasHeight = 1080,
                    const int &frameCount = 200,
                    const qreal &fps = 24.);
    ~Canvas();

    QRectF getPixBoundingRect();
    void selectOnlyLastPressedBox();
    void selectOnlyLastPressedPoint();
    void selectOnlyLastPressedBone();
    void connectPointsFromDifferentPaths(NodePoint *pointSrc,
                                         NodePoint *pointDest);

    void repaintIfNeeded();
    void setCanvasMode(const CanvasMode &mode);
    void startSelectionAtPoint(const QPointF &pos);
    void moveSecondSelectionPoint(const QPointF &pos);
    void setPointCtrlsMode(const CtrlsMode& mode);
    void setCurrentBoxesGroup(BoxesGroup *group);

    void updatePivot();

    void updatePivotIfNeeded();

    void awaitUpdate() {}

    void resetTransormation();
    void fitCanvasToSize();
    void zoomCanvas(const qreal &scaleBy, const QPointF &absOrigin);
    void moveByRel(const QPointF &trans);

    //void updateAfterFrameChanged(const int &currentFrame);

    QSize getCanvasSize();

    void playPreview(const int &minPreviewFrameId,
                     const int &maxPreviewFrameId);

    void clearPreview();

    void centerPivotPosition(const bool &finish = false) { Q_UNUSED(finish); }

    //
    void finishSelectedPointsTransform();
    void finishSelectedBoxesTransform();
    void moveSelectedPointsByAbs(const QPointF &by,
                                 const bool &startTransform);
    void moveSelectedBoxesByAbs(const QPointF &by,
                                const bool &startTransform);
    void moveSelectedBonesByAbs(const QPointF &by,
                                const bool &startTransform);
    void groupSelectedBoxes();

    //void selectAllBoxes();
    void deselectAllBoxes();

    void applyShadowToSelected();

    void selectedPathsUnion();
    void selectedPathsDifference();
    void selectedPathsIntersection();
    void selectedPathsDivision();
    void selectedPathsExclusion();
    void makeSelectedPointsSegmentsCurves();
    void makeSelectedPointsSegmentsLines();

    void updateSelectedPointsAfterCtrlsVisiblityChanged();
    void removeSelectedPointsApproximateAndClearList();
    void centerPivotForSelected();
    void resetSelectedScale();
    void resetSelectedTranslation();
    void resetSelectedRotation();
    void convertSelectedBoxesToPath();
    void convertSelectedPathStrokesToPath();

    void applySampledMotionBlurToSelected();
    void applyLinesEffectToSelected();
    void applyCirclesEffectToSelected();
    void applySwirlEffectToSelected();
    void applyOilEffectToSelected();
    void applyImplodeEffectToSelected();
    void applyDesaturateEffectToSelected();
    void applyColorizeEffectToSelected();
    void applyReplaceColorEffectToSelected();
    void applyContrastEffectToSelected();
    void applyBrightnessEffectToSelected();

    void rotateSelectedBy(const qreal &rotBy,
                          const QPointF &absOrigin,
                          const bool &startTrans);

    QPointF getSelectedBoxesAbsPivotPos();
    bool isSelectionEmpty();

    void ungroupSelectedBoxes();
    void scaleSelectedBy(const qreal& scaleBy,
                         const QPointF &absOrigin,
                         const bool& startTrans);
    void cancelSelectedBoxesTransform();
    void cancelSelectedPointsTransform();
    NodePoint *createNewPointOnLineNearSelected(
            const QPointF &absPos, const bool &adjust,
            const qreal &canvasScaleInv);


    void setSelectedCapStyle(const Qt::PenCapStyle& capStyle);
    void setSelectedJoinStyle(const Qt::PenJoinStyle &joinStyle);
    void setSelectedStrokeWidth(const qreal &strokeWidth, const bool &finish);
    void startSelectedStrokeWidthTransform();
    void startSelectedStrokeColorTransform();
    void startSelectedFillColorTransform();

    void getDisplayedFillStrokeSettingsFromLastSelected(PaintSettings*& fillSetings, StrokeSettings*& strokeSettings);
    void scaleSelectedBy(const qreal &scaleXBy, const qreal &scaleYBy,
                         const QPointF &absOrigin, const bool &startTrans);

    void grabMouseAndTrack();

    void setPartialRepaintRect(QRectF absRect);
    void makePartialRepaintInclude(QPointF pointToInclude);
    void partialRepaintRectToPoint(QPointF point);

    qreal getResolutionFraction();
    void setResolutionFraction(const qreal &percent);

    void renderCurrentFrameToOutput(const RenderInstanceSettings &renderDest);

    void applyCurrentTransformationToSelected();
    QPointF getSelectedPointsAbsPivotPos();
    bool isPointsSelectionEmpty();
    void scaleSelectedPointsBy(const qreal &scaleXBy,
                               const qreal &scaleYBy,
                               const QPointF &absOrigin,
                               const bool &startTrans);
    void rotateSelectedPointsBy(const qreal &rotBy,
                                const QPointF &absOrigin,
                                const bool &startTrans);
    int getPointsSelectionCount();


    void clearPointsSelectionOrDeselect();
    VectorPathEdge *getEdgeAt(QPointF absPos);

    void createLinkBoxForSelected();
    void startSelectedPointsTransform();

    void mergePoints();
    void disconnectPoints();
    void connectPoints();

    void setSelectedFontFamilyAndStyle(
            const QString& family, const QString& style);
    void setSelectedFontSize(const qreal& size);
    void removeSelectedPointsAndClearList();
    void removeSelectedBoxesAndClearList();
    void clearBoxesSelection();
    void clearBonesSelection();
    void removePointFromSelection(MovablePoint *point);
    void removeBoxFromSelection(BoundingBox *box);
    void addPointToSelection(MovablePoint *point);
    void addBoxToSelection(BoundingBox *box);
    void removeBoneFromSelection(Bone *bone);
    void addBoneToSelection(Bone *bone);

    void clearPointsSelection();
    void raiseSelectedBoxesToTop();
    void lowerSelectedBoxesToBottom();
    void raiseSelectedBoxes();
    void lowerSelectedBoxes();

    void selectAndAddContainedPointsToSelection(const QRectF &absRect);
//
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *e);

    bool keyPressEvent(QKeyEvent *event);

    qsptr<BoundingBox> createLink();
    ImageBox* createImageBox(const QString &path);
    ImageSequenceBox* createAnimationBoxForPaths(const QStringList &paths);
    VideoBox* createVideoForPath(const QString &path);
    ExternalLinkBox *createLinkToFileWithPath(const QString &path);
    SingleSound* createSoundForPath(const QString &path);

    void setPreviewing(const bool &bT);
    void setOutputRendering(const bool &bT);

    const CanvasMode &getCurrentCanvasMode() const {
        return mCurrentMode;
    }

    Canvas *getParentCanvas() {
        return this;
    }

    bool SWT_shouldBeVisible(const SWT_RulesCollection &rules,
                             const bool &parentSatisfies,
                             const bool &);

    BoxesGroup *getCurrentBoxesGroup() {
        return mCurrentBoxesGroup;
    }

    void updateCombinedTransform() {}

    QMatrix getCombinedTransform() const {
        QMatrix matrix;
        matrix.reset();
        return matrix;
    }

    QMatrix getRelativeTransformAtCurrentFrame() {
        return QMatrix();
    }

    QPointF mapAbsPosToRel(const QPointF &absPos) {
        return absPos;
    }

    void setIsCurrentCanvas(const bool &bT);

    void scheduleEffectsMarginUpdate() {}

    void renderSk(SkCanvas * const canvas,
                  GrContext * const grContext);

    void setCanvasSize(const int &width, const int &height) {
        if(width == mWidth && height == mHeight) return;
        mWidth = width;
        mHeight = height;
        fitCanvasToSize();
    }

    int getCanvasWidth() const {
        return mWidth;
    }

    QRectF getMaxBoundsRect() const {
        if(mClipToCanvasSize) {
            return QRectF(0., 0.,
                          mWidth, mHeight);
        } else {
            return QRectF(-mWidth, - mHeight,
                          3*mWidth, 3*mHeight);
        }
    }

    int getCanvasHeight() const {
        return mHeight;
    }

    void setMaxFrame(const int &frame);

    ColorAnimator *getBgColorAnimator() {
        return mBackgroundColor.get();
    }

    stdsptr<BoundingBoxRenderData> createRenderData();

    void setupBoundingBoxRenderDataForRelFrameF(const qreal &relFrame,
                                                BoundingBoxRenderData* data) {
        BoxesGroup::setupBoundingBoxRenderDataForRelFrameF(relFrame, data);
        auto canvasData = GetAsPtr(data, CanvasRenderData);
        canvasData->bgColor = QColorToSkColor(mBackgroundColor->getCurrentColor());
        canvasData->canvasHeight = mHeight*mResolutionFraction;
        canvasData->canvasWidth = mWidth*mResolutionFraction;
    }

    bool clipToCanvas() { return mClipToCanvasSize; }

    const Brush *getCurrentBrush() const;
    void setCurrentBrush(const Brush *brush) {
        mCurrentBrush = brush;
    }
protected:
//    void updateAfterCombinedTransformationChanged() {
////        Q_FOREACH(const qsptr<BoundingBox>& child, mChildBoxes) {
////            child->updateCombinedTransformTmp();
////            child->scheduleSoftUpdate();
////        }
//    }

    void setCurrentEndPoint(NodePoint *point);

    NodePoint *getCurrentPoint();

    void handleMovePathMouseRelease();
    void handleMovePointMouseRelease();

    bool isMovingPath();
    void handleRightButtonMousePress(QMouseEvent *event);
    void handleLeftButtonMousePress();
signals:
    void canvasNameChanged(Canvas *, QString);
private slots:
    void emitCanvasNameChanged();
public slots:
    void schedulePivotUpdate();
    void scheduleDisplayedFillStrokeSettingsUpdate();

    void nextPreviewFrame();
    void prp_updateAfterChangedAbsFrameRange(const int &minFrame,
                                             const int &maxFrame);
    void setClipToCanvas(const bool &bT) { mClipToCanvasSize = bT; }
    void setRasterEffectsVisible(const bool &bT) { mRasterEffectsVisible = bT; }
    void setPathEffectsVisible(const bool &bT) { mPathEffectsVisible = bT; }
public:
    void makePointCtrlsSymmetric();
    void makePointCtrlsSmooth();
    void makePointCtrlsCorner();

    void makeSegmentLine();
    void makeSegmentCurve();

    void drawSelectedSk(SkCanvas *canvas,
                        const CanvasMode &currentCanvasMode,
                        const SkScalar &invScale);
    MovablePoint *getPointAtAbsPos(const QPointF &absPos,
                             const CanvasMode &currentMode,
                             const qreal &canvasScaleInv);
    void duplicateSelectedBoxes();
    void clearLastPressedPoint();
    void clearCurrentEndPoint();
    void clearHoveredEdge();
    void applyPaintSettingToSelected(PaintSetting *setting);
    void setSelectedFillColorMode(const ColorMode &mode);
    void setSelectedStrokeColorMode(const ColorMode &mode);
    int getCurrentFrame();
    int getFrameCount();

    SoundComposition *getSoundComposition();

    void updateHoveredBox();
    void updateHoveredPoint();
    void updateHoveredEdge();
    void updateHoveredElements();

    void setLocalPivot(const bool &localPivot) {
        mLocalPivot = localPivot;
        updatePivot();
    }

    void setBonesSelectionEnabled(const bool &bT) {
        mBonesSelectionEnabled = bT;
        clearBonesSelection();
        updatePivot();
    }

    const bool &getPivotLocal() {
        return mLocalPivot;
    }

    int getMaxFrame();

    void beforeCurrentFrameRender();
    void afterCurrentFrameRender();
    //void updatePixmaps();
    CacheHandler& getCacheHandler() {
        return mCacheHandler;
    }

    void setCurrentPreviewContainer(CacheContainer *cont,
                                    const bool &frameEncoded = false);
    void setLoadingPreviewContainer(CacheContainer *cont);

    void setRenderingPreview(const bool &bT);

    bool isPreviewingOrRendering() const {
        return mPreviewing || mRenderingPreview || mRenderingOutput;
    }
    QPointF mapCanvasAbsToRel(const QPointF &pos);
    void applyDiscretePathEffectToSelected();
    void applyDuplicatePathEffectToSelected();
    void applyLengthPathEffectToSelected();
    void applySolidifyPathEffectToSelected();
    void applyDiscreteOutlinePathEffectToSelected();
    void applyDuplicateOutlinePathEffectToSelected();
    void applySumPathEffectToSelected();
    void applyGroupSumPathEffectToSelected();

    const qreal &getFps() const { return mFps; }
    void setFps(const qreal &fps) { mFps = fps; }

    BoundingBox *getBoxAt(const QPointF &absPos) {
        if(mClipToCanvasSize) {
            if(!getMaxBoundsRect().contains(absPos)) return nullptr;
        }
        return BoxesGroup::getBoxAt(absPos);
    }

    void scaleTime(const int &pivotAbsFrame, const qreal &scale) {
        BoxesGroup::scaleTime(pivotAbsFrame, scale);
//        int newAbsPos = qRound(scale*pivotAbsFrame);
//        anim_shiftAllKeys(newAbsPos - pivotAbsFrame);
        setMaxFrame(qRound((mMaxFrame - pivotAbsFrame)*scale));
        mCanvasWindow->setCurrentCanvas(this);
    }

    void changeFpsTo(const qreal& fps) {
        scaleTime(0, fps/mFps);
        setFps(fps);
    }
    void drawTransparencyMesh(SkCanvas *canvas, const SkRect &viewRect);

    bool SWT_isCanvas() { return true; }
    bool handleSelectedCanvasAction(QAction *selectedAction, QWidget* widgetsParent);
    void addCanvasActionToMenu(QMenu *menu);
    void deleteAction();
    void copyAction();
    void pasteAction();
    void cutAction();
    void duplicateAction();
    void selectAllAction();
    void clearSelectionAction();
    void rotateSelectedBoxesStartAndFinish(const qreal &rotBy);
    bool shouldScheduleUpdate() {
        return (isVisibleAndInVisibleDurationRect() ||
               isRelFrameInVisibleDurationRect(anim_mCurrentRelFrame)) &&
               mCurrentPreviewContainerOutdated;
    }

    void renderDataFinished(BoundingBoxRenderData *renderData);
    void prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                               int *lastIdentical,
                                               const int &relFrame);
    void setPickingFromPath(const bool &pickFill,
                            const bool &pickStroke) {
        mPickFillFromPath = pickFill;
        mPickStrokeFromPath = pickStroke;
    }

    void tabletEvent(QTabletEvent *e, const QPointF &absPos);
    QRectF getRelBoundingRectAtRelFrame(const int &);
    void writeBoundingBox(QIODevice *target);
    void readBoundingBox(QIODevice *target);
    bool prp_prevRelFrameWithKey(const int &relFrame, int &prevRelFrame);
    bool prp_nextRelFrameWithKey(const int &relFrame, int &nextRelFrame);
    QMatrix getRelativeTransformAtRelFrame(const int &relFrame) {
        Q_UNUSED(relFrame);
        return QMatrix();
    }
    void applyDiscreteFillPathEffectToSelected();
    void applyDuplicateFillPathEffectToSelected();
    void applySumFillPathEffectToSelected();
    void shiftAllPointsForAllKeys(const int &by);
    void revertAllPointsForAllKeys();
    void shiftAllPoints(const int &by);
    void revertAllPoints();
    void flipSelectedBoxesHorizontally();
    void flipSelectedBoxesVertically();
    int getByteCountPerFrame() {
        return qCeil(mWidth*mResolutionFraction)*
                qCeil(mHeight*mResolutionFraction)*4;
        //return mCurrentPreviewContainer->getByteCount();
    }
    int getMaxPreviewFrame(const int &minFrame, const int &maxFrame);
    void selectedPathsCombine();
    void selectedPathsBreakApart();
    void invertSelectionAction();

    const bool &getRasterEffectsVisible() const {
        return mRasterEffectsVisible;
    }

    const bool &getPathEffectsVisible() const {
        return mPathEffectsVisible;
    }

    void prp_setAbsFrame(const int &frame);

    void moveDurationRectForAllSelected(const int &dFrame);
    void startDurationRectPosTransformForAllSelected();
    void finishDurationRectPosTransformForAllSelected();
    void startMinFramePosTransformForAllSelected();
    void finishMinFramePosTransformForAllSelected();
    void moveMinFrameForAllSelected(const int &dFrame);
    void startMaxFramePosTransformForAllSelected();
    void finishMaxFramePosTransformForAllSelected();
    void moveMaxFrameForAllSelected(const int &dFrame);

    UndoRedoStack *getUndoRedoStack() {
        return mUndoRedoStack.get();
    }

    void blockUndoRedo();
    void unblockUndoRedo();

    template <class T>
    void applyEffectToSelected() {
        Q_FOREACH(const qptr<BoundingBox>& box, mSelectedBoxes) {
            box->addEffect<T>();
        }
    }
    void setParentToLastSelected();
    void clearParentForSelected();
    bool startRotatingAction();
    bool startScalingAction();
    bool startMovingAction();
    void deselectAllBoxesAction();
    void selectAllBoxesAction();
    void selectAllPointsAction();
    bool handleTransormationInputKeyEvent(QKeyEvent *event);
private:
    void setCurrentGroupParentAsCurrentGroup();

    void openTextEditorForTextBox(TextBox *textBox);
    void callUpdateSchedulers();

    bool isShiftPressed();

    bool isShiftPressed(QKeyEvent *event);
    bool isCtrlPressed();
    bool isCtrlPressed(QKeyEvent *event);
    bool isAltPressed();
    bool isAltPressed(QKeyEvent *event);
protected:
    stdsptr<UndoRedoStack> mUndoRedoStack;

    const Brush *mCurrentBrush = nullptr;
    bool mStylusDrawing = false;
    bool mPickFillFromPath = false;
    bool mPickStrokeFromPath = false;

    CacheHandler mCacheHandler;
    bool mUpdateReplaceCache = false;

    sk_sp<SkImage> mRenderImageSk;

    qsptr<ColorAnimator> mBackgroundColor =
            SPtrCreate(ColorAnimator)();

    VectorPath *getPathResultingFromOperation(const bool &unionInterThis,
                                              const bool &unionInterOther);

    void sortSelectedBoxesByZAscending();

    QMatrix mCanvasTransformMatrix;
    qsptr<SoundComposition> mSoundComposition;

    bool mLocalPivot = false;
    bool mBonesSelectionEnabled = false;
    bool mIsCurrentCanvas = true;
    int mMaxFrame = 0;

    qreal mResolutionFraction;

    MainWindow *mMainWindow;
    CanvasWindow *mCanvasWindow;
    QWidget *mCanvasWidget;

    qptr<Circle> mCurrentCircle;
    qptr<Rectangle> mCurrentRectangle;
    qptr<TextBox> mCurrentTextBox;
    qptr<ParticleBox> mCurrentParticleBox;
    qptr<BoxesGroup> mCurrentBoxesGroup;

    stdptr<MovablePoint> mHoveredPoint_d;
    qptr<BoundingBox> mHoveredBox;
    stdptr<VectorPathEdge> mHoveredEdge_d;
    qptr<Bone> mHoveredBone;

    QList<qptr<Bone>> mSelectedBones;
    QList<stdptr<MovablePoint>> mSelectedPoints_d;
    QList<qptr<BoundingBox>> mSelectedBoxes;

    stdptr<MovablePoint> mLastPressedPoint;
    stdptr<NodePoint> mCurrentEndPoint;
    qptr<BoundingBox> mLastPressedBox;
    qptr<Bone> mLastPressedBone;
    stdsptr<PathPivot> mRotPivot;

    bool mTransformationFinishedBeforeMouseRelease = false;

    ValueInput mValueInput;

    bool mXOnlyTransform = false;
    bool mYOnlyTransform = false;

    VectorPathEdge *mCurrentEdge = nullptr;

    bool mPreviewing = false;
    bool mRenderingPreview = false;
    bool mRenderingOutput = false;

    bool mCurrentPreviewContainerOutdated = false;
    stdsptr<CacheContainer> mCurrentPreviewContainer;
    stdsptr<CacheContainer> mLoadingPreviewContainer;

    int mCurrentPreviewFrameId;
    int mMaxPreviewFrameId = 0;

    bool mClipToCanvasSize = false;
    bool mRasterEffectsVisible = true;
    bool mPathEffectsVisible = true;

    bool mIsMouseGrabbing = false;

    bool mDoubleClick = false;
    int mMovesToSkip = 0;

    QColor mFillColor;
    QColor mOutlineColor;

    int mWidth;
    int mHeight;

    qreal mFps = 24.;

    qreal mVisibleWidth;
    qreal mVisibleHeight;
    bool mPivotUpdateNeeded = false;

    bool mFirstMouseMove = false;
    bool mSelecting = false;
//    bool mMoving = false;
    QPointF mLastMouseEventPosRel;
    QPointF mLastMouseEventPosAbs;
    QPointF mLastPressPosAbs;
    QPointF mLastPressPosRel;

    QPointF mCurrentMouseEventPosRel;
    QPointF mCurrentMouseEventPosAbs;
    QPointF mCurrentPressPosAbs;
    QPointF mCurrentPressPosRel;

    QRectF mSelectionRect;
    CanvasMode mCurrentMode = ADD_POINT;

    void setCtrlPointsEnabled(bool enabled);
    void handleMovePointMouseMove();
    void handleMovePathMouseMove();
    void handleAddPointMouseMove();
    void handleMovePathMousePressEvent();
    void handleMovePointMousePressEvent();
    void handleAddPointMouseRelease();
    void handleAddPointMousePress();

    void updateTransformation();
    void handleMouseRelease();
    QPointF getMoveByValueForEventPos(const QPointF &eventPos);
    void cancelCurrentTransform();
    void releaseMouseAndDontTrack();
    void setLastMouseEventPosAbs(const QPointF &abs);
    void setLastMousePressPosAbs(const QPointF &abs);
    void setCurrentMouseEventPosAbs(const QPointF &abs);
    void setCurrentMousePressPosAbs(const QPointF &abs);
};

#endif // CANVAS_H
