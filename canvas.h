#ifndef CANVAS_H
#define CANVAS_H

#include "Boxes/boxesgroup.h"
#include "Colors/color.h"
#include "fillstrokesettings.h"
#include <QSqlQuery>
#include <QThread>
#include "ctrlpoint.h"
#include "Boxes/textbox.h"

class MainWindow;

class UndoRedo;

class UndoRedoStack;

class Circle;
class ParticleBox;
class Rectangle;
class SoundComposition;

enum CanvasMode : short {
    MOVE_PATH,
    MOVE_POINT,
    ADD_POINT,
    ADD_CIRCLE,
    ADD_RECTANGLE,
    ADD_TEXT,
    ADD_PARTICLE_BOX,
    ADD_PARTICLE_EMITTER,
    PICK_PATH_SETTINGS
};

#include "canvaswidget.h"

class Canvas : public BoxesGroup
{
    Q_OBJECT
public:
    explicit Canvas(FillStrokeSettingsWidget *fillStrokeSettings,
                    CanvasWidget *canvasWidget,
                    int canvasWidth = 1920,
                    int canvasHeight = 1080,
                    const int &frameCount = 200);
    ~Canvas();

    QRectF getPixBoundingRect();
    void selectOnlyLastPressedBox();
    void selectOnlyLastPressedPoint();
    void connectPointsFromDifferentPaths(PathPoint *pointSrc,
                                         PathPoint *pointDest);

    void repaintIfNeeded();
    void setCanvasMode(CanvasMode mode);
    void startSelectionAtPoint(QPointF pos);
    void moveSecondSelectionPoint(QPointF pos);
    void setPointCtrlsMode(CtrlsMode mode);
    void setCurrentBoxesGroup(BoxesGroup *group);

    void updatePivot();

    void schedulePivotUpdate();
    void updatePivotIfNeeded();

    void awaitUpdate() {}

    int prp_saveToSql(QSqlQuery *query, const int &parentId = 0);

    void resetTransormation();
    void fitCanvasToSize();
    bool processFilteredKeyEvent(QKeyEvent *event);
    void zoomCanvas(const qreal &scaleBy, const QPointF &absOrigin);
    void moveByRel(const QPointF &trans);

    void updateAfterFrameChanged(const int &currentFrame);

    void renderCurrentFrameToQImage(QImage *frame);

    QSize getCanvasSize();

    void playPreview(const int &minPreviewFrameId,
                     const int &maxPreviewFrameId);

    void renderCurrentFrameToPreview();

    QMatrix getCombinedRenderTransform();

    void clearPreview();

    void centerPivotPosition(bool finish = false) { Q_UNUSED(finish); }
    bool processUnfilteredKeyEvent(QKeyEvent *event);

    //
    void finishSelectedPointsTransform();
    void finishSelectedBoxesTransform();
    void moveSelectedPointsByAbs(const QPointF &by,
                                 const bool &startTransform);
    void moveSelectedBoxesByAbs(const QPointF &by,
                                const bool &startTransform);
    BoxesGroup *groupSelectedBoxes();

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
    void applyAlphaMatteToSelected();

    void rotateSelectedBy(const qreal &rotBy,
                          const QPointF &absOrigin,
                          const bool &startTrans);

    QPointF getSelectedBoxesAbsPivotPos();
    bool isSelectionEmpty();

    void ungroupSelected();
    void scaleSelectedBy(qreal scaleBy, QPointF absOrigin, bool startTrans);
    void cancelSelectedBoxesTransform();
    void cancelSelectedPointsTransform();
    PathPoint *createNewPointOnLineNearSelected(const QPointF &absPos,
                                                const bool &adjust,
                                                const qreal &canvasScaleInv);

    void setSelectedFillGradient(Gradient* gradient, bool finish);
    void setSelectedStrokeGradient(Gradient* gradient, bool finish);
    void setSelectedFillFlatColor(Color color, bool finish);
    void setSelectedStrokeFlatColor(Color color, bool finish);

    void setSelectedCapStyle(Qt::PenCapStyle capStyle);
    void setSelectedJoinStyle(Qt::PenJoinStyle joinStyle);
    void setSelectedStrokeWidth(qreal strokeWidth, bool finish);
    void startSelectedStrokeWidthTransform();
    void startSelectedStrokeColorTransform();
    void startSelectedFillColorTransform();

    void setDisplayedFillStrokeSettingsFromLastSelected();
    void scaleSelectedBy(qreal scaleXBy, qreal scaleYBy,
                         QPointF absOrigin, bool startTrans);
    void updateInputValue();
    void clearAndDisableInput();

    void grabMouseAndTrack();

    void setPartialRepaintRect(QRectF absRect);
    void makePartialRepaintInclude(QPointF pointToInclude);
    void partialRepaintRectToPoint(QPointF point);

    void setEffectsPaintEnabled(const bool &bT);
    bool effectsPaintEnabled();

    qreal getResolutionFraction();
    void setResolutionFraction(const qreal &percent);

    void updateRenderImageSize();

    QMatrix getCombinedFinalRenderTransform();
    void renderCurrentFrameToOutput(const QString &renderDest);
    void drawListItem(QPainter *p, qreal drawX, qreal drawY, qreal maxY);
    void showContextMenu(QPoint globalPos);

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
    void removePointFromSelection(MovablePoint *point);
    void removeBoxFromSelection(BoundingBox *box);
    void addPointToSelection(MovablePoint *point);
    void addBoxToSelection(BoundingBox *box);
    void clearPointsSelection();
    void raiseSelectedBoxesToTop();
    void lowerSelectedBoxesToBottom();
    void raiseSelectedBoxes();
    void lowerSelectedBoxes();

    void selectAndAddContainedPointsToSelection(QRectF absRect);
//
    void paintEvent(QPainter *p);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);

    void keyPressEvent(QKeyEvent *event);
    void drawPreviewPixmap(QPainter *p);
    void createAnimationBoxForPaths(const QStringList &paths);
    void createVideoForPath(const QString &path);

    void setPreviewing(bool bT);
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

    QMatrix getCombinedTransform() const { return QMatrix(); }
    QMatrix getRelativeTransform() const { return QMatrix(); }
    QPointF mapAbsPosToRel(const QPointF &absPos) {
        return absPos;
    }

    void setIsCurrentCanvas(const bool &bT);

    void scheduleEffectsMarginUpdate() {}

    void addChildAwaitingUpdate(BoundingBox *child);

protected:
    void updateAfterCombinedTransformationChanged() {
//        foreach(BoundingBox *child, mChildBoxes) {
//            child->updateCombinedTransformTmp();
//            child->scheduleSoftUpdate();
//        }
    }

    void setCurrentEndPoint(PathPoint *point);

    PathPoint *getCurrentPoint();

    void handleMovePathMouseRelease();
    void handleMovePointMouseRelease();

    bool isMovingPath();
    bool handleKeyPressEventWhileMouseGrabbing(QKeyEvent *event);
    void handleRightButtonMousePress(QMouseEvent *event);
    void handleLeftButtonMousePress();
signals:
private slots:
    void nextPreviewFrame();
public:
    void makePointCtrlsSymmetric();
    void makePointCtrlsSmooth();
    void makePointCtrlsCorner();

    void makeSegmentLine();
    void makeSegmentCurve();

    BoundingBox *createLink(BoxesGroup *parent);
    void createImageBox(const QString &path);
    void drawSelected(QPainter *p, const CanvasMode &currentCanvasMode);
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

    void processUpdate() {
        foreach(const QSharedPointer<BoundingBox> &child,
                mUpdateChildrenAwaitingUpdate) {
            child->processUpdate();
        }
    }
    void createSoundForPath(const QString &path);

    void updateHoveredBox();
    void updateHoveredPoint();
    void updateHoveredEdge();
    void updateHoveredElements();

    void setLocalPivot(const bool &localPivot) {
        mLocalPivot = localPivot;
        updatePivot();
    }

    const bool &getPivotLocal() {
        return mLocalPivot;
    }

    void clearCurrentPreviewImage();
    int getMaxFrame();

    void beforeCurrentFrameRender();
    void afterCurrentFrameRender();
    void beforeUpdate();
    void afterUpdate();
    void updatePixmaps();
    CacheHandler *getCacheHandler() {
        return &mCacheHandler;
    }

private:
    CacheHandler mCacheHandler;
    bool mUpdateReplaceCache = false;

    QImage mRenderImage;
    QSize mRenderImageSize;
    QColor mRenderBackgroundColor;
    QSharedPointer<ColorAnimator> mBackgroundColor =
            (new ColorAnimator())->ref<ColorAnimator>();

    void scheduleUpdate() {}
    VectorPath *getPathResultingFromOperation(const bool &unionInterThis,
                                              const bool &unionInterOther);

    void sortSelectedBoxesByZAscending();

    QMatrix mCanvasTransformMatrix;
    SoundComposition *mSoundComposition;

    MovablePoint *mHoveredPoint = NULL;
    BoundingBox *mHoveredBox = NULL;
    VectorPathEdge *mHoveredEdge = NULL;

    QList<MovablePoint*> mSelectedPoints;
    QList<BoundingBox*> mSelectedBoxes;

    bool mLocalPivot = false;
    bool mIsCurrentCanvas = true;
    int mMaxFrame = 0;

    bool mGlobalPivotVisible = true;
    bool mEffectsPaintEnabled;
    qreal mResolutionFraction;

    CanvasWidget *mCanvasWidget;

    QSize mImageSize;

    Circle *mCurrentCircle = NULL;
    Rectangle *mCurrentRectangle = NULL;
    TextBox *mCurrentTextBox = NULL;
    ParticleBox *mCurrentParticleBox = NULL;

    bool mTransformationFinishedBeforeMouseRelease = false;
    QString mInputText;
    qreal mInputTransformationValue = 0.;
    bool mInputTransformationEnabled = false;

    bool mXOnlyTransform = false;
    bool mYOnlyTransform = false;

    VectorPathEdge *mCurrentEdge = NULL;

    bool mPreviewing = false;
    bool mRendering = false;
    QImage mCurrentPreviewImg;
    int mCurrentPreviewFrameId;
    int mMaxPreviewFrameId = 0;

    bool mIsMouseGrabbing = false;

    bool mDoubleClick = false;
    int mMovesToSkip = 0;

    Color mFillColor;
    Color mOutlineColor;

    BoxesGroup *mCurrentBoxesGroup;

    int mWidth;
    int mHeight;

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
    PathPoint *mCurrentEndPoint = NULL;
    BoundingBox *mLastPressedBox = NULL;
    void setCtrlPointsEnabled(bool enabled);
    PathPivot *mRotPivot;
    void handleMovePointMouseMove();
    void handleMovePathMouseMove();
    void handleAddPointMouseMove();
    void handleMovePathMousePressEvent();
    void handleMovePointMousePressEvent();
    void handleAddPointMouseRelease();

    QList<QImage> mPreviewFrames;
    void updateTransformation();
    void handleMouseRelease();
    QPointF getMoveByValueForEventPos(QPointF eventPos);
    void cancelCurrentTransform();
    void releaseMouseAndDontTrack();
    void setLastMouseEventPosAbs(const QPoint &abs);
    void setLastMousePressPosAbs(const QPoint &abs);
    void setCurrentMouseEventPosAbs(const QPoint &abs);
    void setCurrentMousePressPosAbs(const QPoint &abs);
};

#endif // CANVAS_H
