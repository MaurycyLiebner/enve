#ifndef CANVAS_H
#define CANVAS_H

#include "Boxes/boxesgroup.h"
#include "Colors/color.h"
#include <QThread>
#include "Boxes/rendercachehandler.h"
#include "skiaincludes.h"
#include "valueinput.h"
#include "drawpath.h"

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

#define getAtIndexOrGiveNull(index, list) (( (index) >= (list).count() || (index) < 0 ) ? NULL : (list).at( (index) ))

#define Q_FOREACHInverted(item, list) item = getAtIndexOrGiveNull((list).count() - 1, (list)); \
    for(int i = (list).count() - 1; i >= 0; i--, item = getAtIndexOrGiveNull(i, (list)) )
#define Q_FOREACHInverted2(item, list) for(int i = (list).count() - 1; i >= 0; i--) { \
            item = getAtIndexOrGiveNull(i, (list));

enum CtrlsMode : short;

enum CanvasMode : short {
    MOVE_PATH,
    MOVE_POINT,
    ADD_POINT,
    DRAW_PATH,
    ADD_DRAW_PATH_NODE,
    ADD_CIRCLE,
    ADD_RECTANGLE,
    ADD_TEXT,
    ADD_PARTICLE_BOX,
    ADD_PARTICLE_EMITTER,
    PICK_PATH_SETTINGS,
    ADD_PAINT_BOX,
    PAINT_MODE,
    ADD_BONE
};

#include "canvaswindow.h"

extern bool zLessThan(BoundingBox *box1, BoundingBox *box2);

struct CanvasRenderData : public BoxesGroupRenderData {
    CanvasRenderData(BoundingBox *parentBoxT);
    void renderToImage();
    SkScalar canvasWidth;
    SkScalar canvasHeight;
    SkColor bgColor;
protected:
    void drawSk(SkCanvas *canvas);

    void updateRelBoundingRect();
};

extern bool boxesZSort(BoundingBox *box1, BoundingBox *box2);
class Canvas : public BoxesGroup
{
    Q_OBJECT
public:
    explicit Canvas(FillStrokeSettingsWidget *fillStrokeSettings,
                    CanvasWindow *canvasWidget,
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
    void setPointCtrlsMode(CtrlsMode mode);
    void setCurrentBoxesGroup(BoxesGroup *group);

    void updatePivot();

    void schedulePivotUpdate();
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

    void applyBlurToSelected();
    void applyBrushEffectToSelected();
    void applyLinesEffectToSelected();
    void applyCirclesEffectToSelected();
    void applySwirlEffectToSelected();
    void applyOilEffectToSelected();
    void applyImplodeEffectToSelected();
    void applyDesaturateEffectToSelected();
    void applyColorizeEffectToSelected();
    void applyReplaceColorEffectToSelected();

    void rotateSelectedBy(const qreal &rotBy,
                          const QPointF &absOrigin,
                          const bool &startTrans);

    QPointF getSelectedBoxesAbsPivotPos();
    bool isSelectionEmpty();

    void ungroupSelectedBoxes();
    void scaleSelectedBy(qreal scaleBy, QPointF absOrigin, bool startTrans);
    void cancelSelectedBoxesTransform();
    void cancelSelectedPointsTransform();
    NodePoint *createNewPointOnLineNearSelected(const QPointF &absPos,
                                                const bool &adjust,
                                                const qreal &canvasScaleInv);


    void setSelectedCapStyle(Qt::PenCapStyle capStyle);
    void setSelectedJoinStyle(Qt::PenJoinStyle joinStyle);
    void setSelectedStrokeWidth(qreal strokeWidth, const bool &finish);
    void startSelectedStrokeWidthTransform();
    void startSelectedStrokeColorTransform();
    void startSelectedFillColorTransform();

    void setDisplayedFillStrokeSettingsFromLastSelected();
    void scaleSelectedBy(qreal scaleXBy, qreal scaleYBy,
                         QPointF absOrigin, bool startTrans);

    void grabMouseAndTrack();

    void setPartialRepaintRect(QRectF absRect);
    void makePartialRepaintInclude(QPointF pointToInclude);
    void partialRepaintRectToPoint(QPointF point);

    qreal getResolutionFraction();
    void setResolutionFraction(const qreal &percent);

    void renderCurrentFrameToOutput(const QString &renderDest);

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

    void setSelectedFontFamilyAndStyle(QString family, QString style);
    void setSelectedFontSize(qreal size);
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

    void selectAndAddContainedPointsToSelection(QRectF absRect);
//
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *e);

    bool keyPressEvent(QKeyEvent *event);

    ImageSequenceBox *createAnimationBoxForPaths(const QStringList &paths);
    VideoBox *createVideoForPath(const QString &path);

    void setPreviewing(const bool &bT);
    void setOutputRendering(const bool &bT);
    void createLinkToFileWithPath(const QString &path);

    const CanvasMode &getCurrentCanvasMode() const {
        return mCurrentMode;
    }

    Canvas *getParentCanvas() {
        return this;
    }

    bool isPreviewing() { return mPreviewing; }

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

    void renderSk(SkCanvas *canvas);

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
        return mBackgroundColor.data();
    }

    BoundingBoxRenderData *createRenderData() {
        return new CanvasRenderData(this);
    }

    void setupBoundingBoxRenderDataForRelFrame(const int &relFrame,
                                               BoundingBoxRenderData *data) {
        BoxesGroup::setupBoundingBoxRenderDataForRelFrame(relFrame,
                                                          data);
        CanvasRenderData *canvasData = (CanvasRenderData*)data;
        canvasData->bgColor = mBackgroundColor->getCurrentColor().getSkColor();
        canvasData->canvasHeight = mHeight*mResolutionFraction;
        canvasData->canvasWidth = mWidth*mResolutionFraction;
    }

    bool clipToCanvas() { return mClipToCanvasSize; }

    Brush *getCurrentBrush();
protected:
//    void updateAfterCombinedTransformationChanged() {
////        Q_FOREACH(BoundingBox *child, mChildBoxes) {
////            child->updateCombinedTransformTmp();
////            child->scheduleSoftUpdate();
////        }
//    }

    void setCurrentEndPoint(NodePoint *point);

    NodePoint *getCurrentPoint();

    void handleMovePathMouseRelease();
    void handleMovePointMouseRelease();

    bool isMovingPath();
    bool handleKeyPressEventWhileMouseGrabbing(QKeyEvent *event);
    void handleRightButtonMousePress(QMouseEvent *event);
    void handleLeftButtonMousePress();
signals:
    void canvasNameChanged(Canvas *, QString);
private slots:
    void emitCanvasNameChanged();
public slots:
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

    BoundingBox *createLink();
    ImageBox *createImageBox(const QString &path);
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
    void applyPaintSettingToSelected(const PaintSetting &setting);
    void setSelectedFillColorMode(const ColorMode &mode);
    void setSelectedStrokeColorMode(const ColorMode &mode);
    int getCurrentFrame();
    int getFrameCount();

    SoundComposition *getSoundComposition();

    SingleSound *createSoundForPath(const QString &path);

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

    void clearCurrentPreviewImage();
    int getMaxFrame();

    void beforeCurrentFrameRender();
    void afterCurrentFrameRender();
    //void updatePixmaps();
    CacheHandler *getCacheHandler() {
        return &mCacheHandler;
    }

    void setCurrentPreviewContainer(CacheContainer *cont);
    void setRendering(const bool &bT);

    bool isPreviewingOrRendering() const {
        return mPreviewing || mRendering;
    }
    QPointF mapCanvasAbsToRel(const QPointF &pos);
    void applyDiscretePathEffectToSelected();
    void applyDuplicatePathEffectToSelected();
    void applySolidifyPathEffectToSelected();
    void applyDiscreteOutlinePathEffectToSelected();
    void applyDuplicateOutlinePathEffectToSelected();
    void applySumPathEffectToSelected();

    const qreal &getFps() const { return mFps; }
    void setFps(const qreal &fps) { mFps = fps; }
    void drawTransparencyMesh(SkCanvas *canvas, const SkRect &viewRect);

    bool SWT_isCanvas() { return true; }
    bool handleSelectedCanvasAction(QAction *selectedAction);
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
        return mCurrentPreviewContainer->getByteCount();
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

    void prp_setAbsFrame(const int &frame) {
        int lastRelFrame = anim_mCurrentRelFrame;
        ComplexAnimator::prp_setAbsFrame(frame);
        CacheContainer *cont =
                mCacheHandler.getRenderContainerAtRelFrame(anim_mCurrentRelFrame);
        if(cont == NULL) {
            mCurrentPreviewContainerOutdated = true;
            bool isInVisRange = isRelFrameInVisibleDurationRect(
                        anim_mCurrentRelFrame);
            if(mUpdateDrawOnParentBox != isInVisRange) {
                if(mUpdateDrawOnParentBox) {
                    mParentGroup->scheduleUpdate();
                } else {
                    scheduleUpdate();
                }
                mUpdateDrawOnParentBox = isInVisRange;
            }
            if(prp_differencesBetweenRelFrames(lastRelFrame,
                                               anim_mCurrentRelFrame)) {
                scheduleUpdate();
            }
        } else {
            setCurrentPreviewContainer(cont);
        }

        Q_FOREACH(const QSharedPointer<BoundingBox> &box, mContainedBoxes) {
            box->prp_setAbsFrame(frame);
        }
    }

protected:
    Brush *mCurrentBrush;
    bool mStylusDrawing = false;
    bool mPickFillFromPath = false;
    bool mPickStrokeFromPath = false;

    CacheHandler mCacheHandler;
    bool mUpdateReplaceCache = false;

    sk_sp<SkImage> mRenderImageSk;

    QSharedPointer<ColorAnimator> mBackgroundColor =
            (new ColorAnimator())->ref<ColorAnimator>();

    VectorPath *getPathResultingFromOperation(const bool &unionInterThis,
                                              const bool &unionInterOther);

    void sortSelectedBoxesByZAscending();

    QMatrix mCanvasTransformMatrix;
    SoundComposition *mSoundComposition = NULL;

    MovablePoint *mHoveredPoint = NULL;
    BoundingBox *mHoveredBox = NULL;
    VectorPathEdge *mHoveredEdge = NULL;
    Bone *mHoveredBone = NULL;

    QList<Bone*> mSelectedBones;
    QList<MovablePoint*> mSelectedPoints;
    QList<BoundingBox*> mSelectedBoxes;

    bool mLocalPivot = false;
    bool mBonesSelectionEnabled = false;
    bool mIsCurrentCanvas = true;
    int mMaxFrame = 0;

    qreal mResolutionFraction;

    CanvasWindow *mCanvasWindow;
    QWidget *mCanvasWidget;

    Circle *mCurrentCircle = NULL;
    Rectangle *mCurrentRectangle = NULL;
    TextBox *mCurrentTextBox = NULL;
    ParticleBox *mCurrentParticleBox = NULL;

    bool mTransformationFinishedBeforeMouseRelease = false;

    ValueInput mValueInput;

    bool mXOnlyTransform = false;
    bool mYOnlyTransform = false;

    VectorPathEdge *mCurrentEdge = NULL;

    bool mPreviewing = false;
    bool mRendering = false;

    bool mCurrentPreviewContainerOutdated = false;
    std::shared_ptr<CacheContainer> mCurrentPreviewContainer;
    int mCurrentPreviewFrameId;
    int mMaxPreviewFrameId = 0;

    bool mClipToCanvasSize = false;
    bool mRasterEffectsVisible = true;
    bool mPathEffectsVisible = true;

    bool mIsMouseGrabbing = false;

    bool mDoubleClick = false;
    int mMovesToSkip = 0;

    Color mFillColor;
    Color mOutlineColor;

    BoxesGroup *mCurrentBoxesGroup;

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
    MovablePoint *mLastPressedPoint = NULL;
    NodePoint *mCurrentEndPoint = NULL;
    BoundingBox *mLastPressedBox = NULL;
    Bone *mLastPressedBone = NULL;
    void setCtrlPointsEnabled(bool enabled);
    PathPivot *mRotPivot;
    void handleMovePointMouseMove();
    void handleMovePathMouseMove();
    void handleAddPointMouseMove();
    void handleMovePathMousePressEvent();
    void handleMovePointMousePressEvent();
    void handleAddPointMouseRelease();
    void handleAddPointMousePress();

    DrawPath mDrawPath;
    void handleDrawPathMousePressEvent();
    void handleAddDrawPathNodeMousePressEvent();
    void handleDrawPathMouseMoveEvent();
    void handleDrawPathMouseReleaseEvent();

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
