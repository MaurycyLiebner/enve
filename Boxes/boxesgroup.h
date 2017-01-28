#ifndef BOXESGROUP_H
#define BOXESGROUP_H
#include "Boxes/boundingbox.h"
#include "Boxes/vectorpath.h"
#include "fillstrokesettings.h"

#define foreachBoxInListInverted(boxesList) BoundingBox *box = getAtIndexOrGiveNull((boxesList).count() - 1, (boxesList)); \
    for(int i = (boxesList).count() - 1; i >= 0; i--, box = getAtIndexOrGiveNull(i, (boxesList)) )



class MainWindow;

class Edge;

class BoxesGroup : public BoundingBox
{
    Q_OBJECT
public:
    BoxesGroup(BoxesGroup *parent);
    BoxesGroup(FillStrokeSettingsWidget *fillStrokeSetting);
    ~BoxesGroup();
    virtual void loadFromSql(int boundingBoxId);

    BoundingBox *createLink(BoxesGroup *parent);
    BoundingBox *createSameTransformationLink(BoxesGroup *parent);

    void setSelectedFontFamilyAndStyle(QString family, QString style);
    void setSelectedFontSize(qreal size);
    void draw(QPainter *p);
//    void render(QPainter *p);
    void drawSelected(QPainter *p, CanvasMode currentCanvasMode);
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
    void selectAllBoxes();
    void deselectAllBoxes();
    void setPointCtrlsMode(CtrlsMode mode);
    void mergePoints();
    void disconnectPoints();
    void connectPoints();
    BoundingBox *getBoxAt(QPointF absPos);
    MovablePoint *getPointAt(QPointF absPos, CanvasMode currentMode);
    void selectAndAddContainedPointsToSelection(QRectF absRect);
    void finishSelectedPointsTransform();
    void addContainedBoxesToSelection(QRectF rect);
    void finishSelectedBoxesTransform();
    void moveSelectedPointsBy(QPointF by, bool startTransform);
    void moveSelectedBoxesBy(QPointF by, bool startTransform);
    BoxesGroup *groupSelectedBoxes();
    void drawBoundingRect(QPainter *p);

    void setIsCurrentGroup(bool bT);

    BoundingBox *getPathAtFromAllAncestors(QPointF absPos);

    void setFillSettings(PaintSettings fillSettings,
                         bool saveUndoRedo = true);
    void setStrokeSettings(StrokeSettings strokeSettings,
                           bool saveUndoRedo = true);
    void setSelectedFillSettings(PaintSettings fillSettings, bool saveUndoRedo = true);
    void setSelectedStrokeSettings(StrokeSettings strokeSettings, bool saveUndoRedo = true);
    void setCurrentFillStrokeSettingsFromBox(BoundingBox *box);

    void rotateSelectedBy(qreal rotBy, QPointF absOrigin, bool startTrans);

    QPointF getSelectedBoxesAbsPivotPos();
    bool isSelectionEmpty();
    void setSelectedPivotAbsPos(QPointF absPos);

    void ungroupSelected();
    void ungroup();

    void scaleSelectedBy(qreal scaleBy, QPointF absOrigin, bool startTrans);
    void cancelSelectedBoxesTransform();

    int saveToSql(QSqlQuery *query, int parentId);
    BoxesGroup *loadChildrenFromSql(int thisBoundingBoxId, bool loadInBox);
    PathPoint *createNewPointOnLineNearSelected(QPointF absPos, bool adjust);
    void saveSelectedToSql(QSqlQuery *query);

    //

    void drawListItem(QPainter *p, qreal drawX, qreal drawY, qreal maxY);
    void drawChildrenListItems(QPainter *p, qreal drawX, qreal drawY, qreal maxY);
    qreal getListItemHeight();
    void handleListItemMousePress(qreal boxesListX,
                                  qreal relX, qreal relY,
                                  QMouseEvent *event);
    void handleChildListItemMousePress(qreal boxesListX,
                                       qreal relX, qreal relY,
                                       qreal y0, QMouseEvent *event);

    bool isCurrentGroup();
    void addChild(BoundingBox *child);
    void addChildToListAt(int index, BoundingBox *child, bool saveUndoRedo = true);
    void updateChildrenId(int firstId, bool saveUndoRedo = true);
    void updateChildrenId(int firstId, int lastId, bool saveUndoRedo = true);
    void removeChild(BoundingBox *child);
    void increaseChildZInList(BoundingBox *child);
    void decreaseChildZInList(BoundingBox *child);
    void bringChildToEndList(BoundingBox *child);
    void bringChildToFrontList(BoundingBox *child);
    void moveChildInList(BoundingBox *child,
                         int from, int to,
                         bool saveUndoRedo = true);
    void updateAfterCombinedTransformationChanged();
    void removeChildFromList(int id, bool saveUndoRedo = true);
    void cancelSelectedPointsTransform();

    void updateAfterFrameChanged(int currentFrame);

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


    void setFillGradient(Gradient* gradient, bool finish);
    void setStrokeGradient(Gradient* gradient, bool finish);
    void setFillFlatColor(Color color, bool finish);
    void setStrokeFlatColor(Color color, bool finish);
    void setFillPaintType(PaintType paintType, Color color,
                                  Gradient* gradient);
    void setStrokePaintType(PaintType paintType, Color color,
                                    Gradient* gradient);
    void setStrokeCapStyle(Qt::PenCapStyle capStyle);
    void setStrokeJoinStyle(Qt::PenJoinStyle joinStyle);
    void setStrokeWidth(qreal strokeWidth, bool finish);

    void startStrokeWidthTransform();
    void startStrokeColorTransform();
    void startFillColorTransform();


    void setSelectedStrokeWidth(qreal strokeWidth, bool finish);

    void startSelectedStrokeWidthTransform();
    void startSelectedStrokeColorTransform();
    void startSelectedFillColorTransform();
    void clearPointsSelectionOrDeselect();
    Edge *getPressedEdge(QPointF absPos);
    void setDisplayedFillStrokeSettingsFromLastSelected();

    void drawChildrenKeysView(QPainter *p,
                              qreal drawY, qreal maxY,
                              qreal pixelsPerFrame,
                              int startFrame, int endFrame);
    void drawKeysView(QPainter *p,
                      qreal drawY, qreal maxY,
                      qreal pixelsPerFrame,
                      int startFrame, int endFrame);
    void scaleSelectedBy(qreal scaleXBy, qreal scaleYBy,
                         QPointF absOrigin, bool startTrans);
    const PaintSettings *getFillSettings();
    const StrokeSettings *getStrokeSettings();

    static bool getCtrlsAlwaysVisible();
    static void setCtrlsAlwaysVisible(bool bT);
    void updateSelectedPointsAfterCtrlsVisiblityChanged();
    void removeSelectedPointsApproximateAndClearList();
    void centerPivotForSelected();
    void resetSelectedScale();
    void resetSelectedTranslation();
    void resetSelectedRotation();
    void convertSelectedBoxesToPath();
    void applyBlurToSelected();
//    void renderFinal(QPainter *p);
    void applyBrushEffectToSelected();
    void applyLinesEffectToSelected();
    void applyCirclesEffectToSelected();
    //void drawPixmap(QPainter *p);
    void updateAllBoxes();
    void handleListItemMouseDoubleClick(qreal boxesListX, qreal relX, qreal relY, QMouseEvent *event);
    void applyCurrentTransformationToSelected();
    QPointF getSelectedPointsAbsPivotPos();
    bool isPointsSelectionEmpty();
    void scaleSelectedPointsBy(qreal scaleXBy, qreal scaleYBy, QPointF absOrigin, bool startTrans);
    void rotateSelectedPointsBy(qreal rotBy, QPointF absOrigin, bool startTrans);
    int getPointsSelectionCount();

    void setSelectedAnimated(bool animated);
    void updateBoundingRect();
    void applyCurrentTransformation();
    void applyShadowToSelected();

    void selectedPathsUnion();
    void selectedPathsDifference();
    void selectedPathsIntersection();
    void selectedPathsDivision();
    void selectedPathsExclusion();
    void makeSelectedPointsSegmentsCurves();
    void makeSelectedPointsSegmentsLines();
    bool relPointInsidePath(QPointF relPos);
//    QPointF getRelCenterPosition();
    void createLinkBoxForSelected();
    void updateEffectsMargin();
protected:
    VectorPath *getPathResultingFromOperation(const bool &unionInterThis,
                                              const bool &unionInterOther);

    void sortSelectedBoxesByZAscending();

    static bool mCtrlsAlwaysVisible;
    FillStrokeSettingsWidget *mFillStrokeSettingsWidget;
    bool mIsCurrentGroup = false;
    QList<MovablePoint*> mSelectedPoints;
    QList<BoundingBox*> mSelectedBoxes;
    QList<BoundingBox*> mChildren;
signals:
    void changeChildZSignal(int, int);
    void removeAnimatedBoundingBoxSignal(BoundingBox*);
    void addAnimatedBoundingBoxSignal(BoundingBox*);
};

#endif // BOXESGROUP_H
