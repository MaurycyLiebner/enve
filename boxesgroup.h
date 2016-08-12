#ifndef BOXESGROUP_H
#define BOXESGROUP_H
#include "boundingbox.h"
#include "vectorpath.h"
#include "fillstrokesettings.h"

#define getAtIndexOrGiveNull(index, list) (( (index) >= (list).count() || (index) < 0 ) ? NULL : (list).at( (index) ))

#define foreachBoxInListInverted(boxesList) BoundingBox *box = getAtIndexOrGiveNull((boxesList).count() - 1, (boxesList)); \
    for(int i = (boxesList).count() - 1; i >= 0; i--, box = getAtIndexOrGiveNull(i, (boxesList)) )

#define foreachBoxInList(boxesList) foreach(BoundingBox *box, (boxesList))

class MainWindow;

class BoxesGroup : public BoundingBox
{
public:
    BoxesGroup(FillStrokeSettingsWidget *fillStrokeSetting, BoundingBox *parent);
    BoxesGroup(FillStrokeSettingsWidget *fillStrokeSetting, MainWindow *parent);

    bool pointInsidePath(QPointF absPos);
    QRectF getBoundingRect();
    void draw(QPainter *p);
    void drawSelected(QPainter *p, CanvasMode currentCanvasMode);
    void removeSelectedPointsAndClearList();
    void removeSelectedBoxesAndClearList();
    void removeChild(BoundingBox *box);
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
    bool isShiftPressed();
    bool isCtrlPressed();
    BoxesGroup *groupSelectedBoxes();
    void drawBoundingRect(QPainter *p);

    void setIsCurrentGroup(bool bT);

    BoundingBox *getBoxAtFromAllAncestors(QPointF absPos);

    void setSelectedFillStrokeSettings(PaintSettings fillSettings,
                                       StrokeSettings strokeSettings);
    void setFillStrokeSettings(PaintSettings fillSettings,
                               StrokeSettings strokeSettings);
    void setFillSettings(PaintSettings fillSettings);
    void setStrokeSettings(StrokeSettings strokeSettings);
    void setSelectedFillSettings(PaintSettings fillSettings);
    void setSelectedStrokeSettings(StrokeSettings strokeSettings);
protected:
    FillStrokeSettingsWidget *mFillStrokeSettingsWidget;
    bool mIsCurrentGroup = false;
    QList<MovablePoint*> mSelectedPoints;
    QList<BoundingBox*> mSelectedBoxes;
};

#endif // BOXESGROUP_H
