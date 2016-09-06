#ifndef BOXESGROUP_H
#define BOXESGROUP_H
#include "boundingbox.h"
#include "vectorpath.h"
#include "fillstrokesettings.h"
#include "bone.h"

#define getAtIndexOrGiveNull(index, list) (( (index) >= (list).count() || (index) < 0 ) ? NULL : (list).at( (index) ))

#define foreachBoxInListInverted(boxesList) BoundingBox *box = getAtIndexOrGiveNull((boxesList).count() - 1, (boxesList)); \
    for(int i = (boxesList).count() - 1; i >= 0; i--, box = getAtIndexOrGiveNull(i, (boxesList)) )

class MainWindow;

class BoxesGroup : public BoundingBox
{
public:
    BoxesGroup(FillStrokeSettingsWidget *fillStrokeSetting, BoxesGroup *parent);
    BoxesGroup(FillStrokeSettingsWidget *fillStrokeSetting, MainWindow *parent);
    BoxesGroup(int boundingBoxId,
               FillStrokeSettingsWidget *fillStrokeSetting, BoxesGroup *parent);

    bool pointInsidePath(QPointF absPos);
    QRectF getBoundingRect();
    void draw(QPainter *p);
    void render(QPainter *p);
    void drawSelected(QPainter *p, CanvasMode currentCanvasMode);
    void removeSelectedPointsAndClearList();
    void removeSelectedBoxesAndClearList();
    void clearBoxesSelection();
    void removePointFromSelection(MovablePoint *point);
    void removeBoxFromSelection(BoundingBox *box);
    void addPointToSelection(MovablePoint *point);
    void addBoxToSelection(BoundingBox *box);
    void clearPointsSelection();
    void bringSelectedBoxesToFront();
    void bringSelectedBoxesToEnd();
    void moveSelectedBoxesUp();
    void moveSelectedBoxesDown();
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

    void startStrokeTransform();
    void startFillTransform();
    void finishStrokeTransform();
    void finishFillTransform();

    void startSelectedStrokeTransform();
    void startSelectedFillTransform();
    void finishSelectedStrokeTransform();
    void finishSelectedFillTransform();

    void rotateSelectedBy(qreal rotBy, QPointF absOrigin, bool startTrans);

    QPointF getSelectedPivotPos();
    bool isSelectionEmpty();
    void setSelectedPivotAbsPos(QPointF absPos);

    void ungroupSelected();
    void ungroup();
    void updatePivotPosition();

    void select();
    void scaleSelectedBy(qreal scaleBy, QPointF absOrigin, bool startTrans);
    void cancelSelectedBoxesTransform();

    int saveToSql(int parentId);
    BoxesGroup *loadChildrenFromSql(QString thisBoundingBoxId, bool loadInBox);
    PathPoint *createNewPointOnLineNearSelected(QPointF absPos);
    void saveSelectedToSql();

    //

    void drawListItem(QPainter *p, qreal drawX, qreal drawY, qreal maxY,
                      qreal pixelsPerFrame, int startFrame, int endFrame);
    void drawChildren(QPainter *p,
                      qreal drawX, qreal drawY,
                      qreal minY, qreal maxY, qreal pixelsPerFrame,
                      int startFrame, int endFrame);
    qreal getListItemHeight();
    void handleListItemMousePress(qreal relX, qreal relY);
    void handleChildListItemMousePress(qreal relX, qreal relY, qreal y0);

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
    void moveChildInList(int from, int to, bool saveUndoRedo = true);
    void updateAfterCombinedTransformationChanged();
    void clearAll();
    void removeChildFromList(int id, bool saveUndoRedo = true);
    Bone *getBoneAt(QPointF absPos);
    void attachToBone(Bone *parentBone, CanvasMode currentCanvasMode);
    void detachFromBone(CanvasMode currentCanvasMode);
    void cancelSelectedPointsTransform();
    Bone *boneFromZIndex(int index);

    void attachToBoneFromSqlZId();
    void updateAfterFrameChanged(int currentFrame);
    QrealKey *getKeyAtPos(qreal relX, qreal relY, qreal y0);
    void getKeysInRect(QRectF selectionRect, qreal y0, QList<QrealKey *> *keysList);
protected:
    FillStrokeSettingsWidget *mFillStrokeSettingsWidget;
    bool mIsCurrentGroup = false;
    QList<MovablePoint*> mSelectedPoints;
    QList<BoundingBox*> mSelectedBoxes;
    QList<BoundingBox*> mChildren;
};

#endif // BOXESGROUP_H
