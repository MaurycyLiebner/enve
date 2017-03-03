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

class Rectangle;
class SoundComposition;

enum CanvasMode : short {
    MOVE_PATH,
    MOVE_POINT,
    ADD_POINT,
    ADD_CIRCLE,
    ADD_RECTANGLE,
    ADD_TEXT,
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
    void connectPointsFromDifferentPaths(PathPoint *pointSrc, PathPoint *pointDest);

    void repaintIfNeeded();
    void setCanvasMode(CanvasMode mode);
    void startSelectionAtPoint(QPointF pos);
    void moveSecondSelectionPoint(QPointF pos);
    void setPointCtrlsMode(CtrlsMode mode);
    QPointF scaleDistancePointByCurrentScale(QPointF point);
    void setCurrentBoxesGroup(BoxesGroup *group);

    void updatePivot();

    void schedulePivotUpdate();
    void updatePivotIfNeeded();
    void setPivotPositionForSelected();

    void awaitUpdate() {}
    void scheduleAwaitUpdate(const bool &) {}

    void saveToSql(QSqlQuery *query);
    void loadAllBoxesFromSql(bool loadInBox);
    void clearAll();
    void resetTransormation();
    void fitCanvasToSize();
    bool processFilteredKeyEvent(QKeyEvent *event);
    void scale(qreal scaleXBy, qreal scaleYBy, QPointF absOrigin);
    void scale(qreal scaleBy, QPointF absOrigin);
    void moveByRel(QPointF trans);

    void updateAfterFrameChanged(int currentFrame);

    void renderCurrentFrameToQImage(QImage *frame);
    void renderFinalCurrentFrameToQImage(QImage *frame);

    QSize getCanvasSize();

    void playPreview();

    void renderCurrentFrameToPreview();

    QMatrix getCombinedRenderTransform();

    void clearPreview();

    void centerPivotPosition(bool finish = false) { Q_UNUSED(finish); }
    bool processUnfilteredKeyEvent(QKeyEvent *event);

    //
    void finishSelectedPointsTransform();
    void finishSelectedBoxesTransform();
    void moveSelectedPointsByAbs(QPointF by, bool startTransform);
    void moveSelectedBoxesByAbs(QPointF by, bool startTransform);
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

    void applyBlurToSelected();
    void applyBrushEffectToSelected();
    void applyLinesEffectToSelected();
    void applyCirclesEffectToSelected();
    void applySwirlEffectToSelected();
    void applyOilEffectToSelected();
    void applyImplodeEffectToSelected();
    void applyDesaturateEffectToSelected();

    void rotateSelectedBy(qreal rotBy, QPointF absOrigin, bool startTrans);

    QPointF getSelectedBoxesAbsPivotPos();
    bool isSelectionEmpty();
    void setSelectedPivotAbsPos(QPointF absPos);

    void ungroupSelected();
    void scaleSelectedBy(qreal scaleBy, QPointF absOrigin, bool startTrans);
    void cancelSelectedBoxesTransform();
    void cancelSelectedPointsTransform();
    PathPoint *createNewPointOnLineNearSelected(QPointF absPos, bool adjust);
    void saveSelectedToSql(QSqlQuery *query);

    void setSelectedFillGradient(Gradient* gradient, bool finish);
    void setSelectedStrokeGradient(Gradient* gradient, bool finish);
    void setSelectedFillFlatColor(Color color, bool finish);
    void setSelectedStrokeFlatColor(Color color, bool finish);
    void setSelectedFillPaintType(PaintType paintType, Color color,
                              Gradient* gradient);
    void setSelectedStrokePaintType(PaintType paintType, Color color,
                                Gradient* gradient);
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

    qreal getCurrentCanvasScale();

    void grabMouseAndTrack();

    void setPartialRepaintRect(QRectF absRect);
    void makePartialRepaintInclude(QPointF pointToInclude);
    void partialRepaintRectToPoint(QPointF point);
    void setHighQualityPaint(const bool &bT);
    bool highQualityPaint();

    void setEffectsPaintEnabled(const bool &bT);
    bool effectsPaintEnabled();

    qreal getResolutionPercent();
    void setResolutionPercent(const qreal &percent);

    void updateRenderRect();
    QMatrix getCombinedFinalRenderTransform();
    void renderCurrentFrameToOutput(QString renderDest);
    void drawListItem(QPainter *p, qreal drawX, qreal drawY, qreal maxY);
    void showContextMenu(QPoint globalPos);
    void updateAllBoxes();

    void applyCurrentTransformationToSelected();
    QPointF getSelectedPointsAbsPivotPos();
    bool isPointsSelectionEmpty();
    void scaleSelectedPointsBy(qreal scaleXBy, qreal scaleYBy, QPointF absOrigin, bool startTrans);
    void rotateSelectedPointsBy(qreal rotBy, QPointF absOrigin, bool startTrans);
    int getPointsSelectionCount();


    void clearPointsSelectionOrDeselect();
    Edge *getEdgeAt(QPointF absPos);

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
    void renderFinal(QPainter *p);
    void createAnimationBoxForPaths(const QStringList &paths);
    void createVideoForPath(const QString &path);

    void setPreviewing(bool bT);
    void createLinkToFileWithPath(const QString &path);

    QRectF getRenderRect();

    const CanvasMode &getCurrentCanvasMode() const {
        return mCurrentMode;
    }

    Canvas *getParentCanvas() {
        return this;
    }

    bool isPreviewing() { return mPreviewing; }

    bool SWT_satisfiesRule(const SWT_RulesCollection &rules,
                           const bool &parentSatisfies);

    BoxesGroup *getCurrentBoxesGroup() {
        return mCurrentBoxesGroup;
    }

    void updateCombinedTransform() {
        updateAfterCombinedTransformationChanged();
    }

    void setIsCurrentCanvas(const bool &bT);

    void scheduleEffectsMarginUpdate() {}
protected:
//    void updateAfterCombinedTransformationChanged();

    void setCurrentEndPoint(PathPoint *point);

    PathPoint *getCurrentPoint();

    void handleMovePathMouseRelease(QPointF pos);
    void handleMovePointMouseRelease(QPointF pos);

    bool isMovingPath();
    bool handleKeyPressEventWhileMouseGrabbing(QKeyEvent *event);
    void handleRightButtonMousePress(QMouseEvent *event);
    void handleLeftButtonMousePress(QMouseEvent *event);
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
    MovablePoint *getPointAt(const QPointF &absPos,
                             const CanvasMode &currentMode);
    void duplicateSelectedBoxes();
    void clearLastPressedPoint();
    void clearCurrentEndPoint();
    void clearHoveredEdge();
    void applyPaintSettingToSelected(const PaintSetting &setting);
    void setSelectedFillColorMode(const ColorMode &mode);
    void setSelectedStrokeColorMode(const ColorMode &mode);
    int getCurrentFrame();
    int getMinFrame();
    int getMaxFrame();

    SoundComposition *getSoundComposition();
private:
    VectorPath *getPathResultingFromOperation(const bool &unionInterThis,
                                              const bool &unionInterOther);

    void sortSelectedBoxesByZAscending();

    SoundComposition *mSoundComposition;

    MovablePoint *mHoveredPoint = NULL;
    BoundingBox *mHoveredBox = NULL;
    Edge *mHoveredEdge = NULL;

    QList<MovablePoint*> mSelectedPoints;
    QList<BoundingBox*> mSelectedBoxes;

    bool mIsCurrentCanvas = true;
    int mCurrentFrame = 0;
    int mMinFrame = 0;
    int mMaxFrame = 0;

    bool mPivotVisibleDuringPointEdit = true;
    bool mEffectsPaintEnabled;
    qreal mResolutionPercent;

    CanvasWidget *mCanvasWidget;

    QRectF mCanvasRect;
    QRectF mRenderRect;

    Circle *mCurrentCircle = NULL;
    Rectangle *mCurrentRectangle = NULL;
    TextBox *mCurrentTextBox = NULL;

    bool mTransformationFinishedBeforeMouseRelease = false;
    QString mInputText;
    qreal mInputTransformationValue = 0.;
    bool mInputTransformationEnabled = false;

    bool mXOnlyTransform = false;
    bool mYOnlyTransform = false;

    Edge *mCurrentEdge = NULL;

    bool mPreviewing = false;
    QImage *mCurrentPreviewImg = NULL;
    int mCurrentPreviewFrameId;

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
    QPoint mLastMouseEventPos;
    QPointF mLastPressPos;
    QRectF mSelectionRect;
    CanvasMode mCurrentMode = ADD_POINT;
    MovablePoint *mLastPressedPoint = NULL;
    PathPoint *mCurrentEndPoint = NULL;
    BoundingBox *mLastPressedBox = NULL;
    void setCtrlPointsEnabled(bool enabled);
    PathPivot *mRotPivot;
    void handleMovePointMouseMove(QPointF eventPos);
    void handleMovePathMouseMove(QPointF eventPos);
    void handleAddPointMouseMove(QPointF eventPos);
    void handleMovePathMousePressEvent();
    void handleAddPointMouseRelease();

    QList<QImage*> mPreviewFrames;
    void updateTransformation();
    void handleMouseRelease(QPointF eventPos);
    QPointF getMoveByValueForEventPos(QPointF eventPos);
    void cancelCurrentTransform();
    void releaseMouseAndDontTrack();
};

#endif // CANVAS_H
