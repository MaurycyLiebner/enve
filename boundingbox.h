#ifndef CHILDPARENT_H
#define CHILDPARENT_H
#include <QMatrix>
#include "connectedtomainwindow.h"

class UndoRedo;

class Canvas;

class UndoRedoStack;

class MovablePoint;

enum CanvasMode : short;

enum BoundingBoxType {
    TYPE_VECTOR_PATH,
    TYPE_GROUP,
    TYPE_CANVAS
};

class BoundingBox : public ConnectedToMainWindow
{
public:
    BoundingBox(BoundingBox *parent, BoundingBoxType type);
    BoundingBox(MainWindow *window, BoundingBoxType type);

    virtual void updatePivotPosition() {}
    virtual bool isContainedIn(QRectF absRect) {}
    virtual QRectF getBoundingRect() {}

    virtual void draw(QPainter *p) {}
    virtual void drawSelected(QPainter *p, CanvasMode currentCanvasMode) {}

    QMatrix getCombinedTransform();

    QPointF getTranslation();

    void scale(qreal scaleBy, QPointF absOrigin);
    void scale(qreal scaleXBy, qreal scaleYBy, QPointF absOrigin);

    void rotateBy(qreal rot, QPointF absOrigin);
    void moveBy(qreal dx, qreal dy);

    void setTransformation(QMatrix transMatrix);

    QPointF getAbsolutePos();

    void updateCombinedTransform();
    void moveBy(QPointF trans);

    void startTransform();
    void finishTransform();

    virtual void addChild(BoundingBox *child);
    virtual void removeChild(BoundingBox *child);

    virtual bool pointInsidePath(QPointF absPos) {}
    virtual MovablePoint *getPointAt(QPointF absPos, CanvasMode currentMode) {}

    void moveUp();
    void moveDown();
    void bringToFront();
    void bringToEnd();

    void increaseChildZInList(BoundingBox *child);
    void decreaseChildZInList(BoundingBox *child);
    void bringChildToEndList(BoundingBox *child);
    void bringChildToFrontList(BoundingBox *child);

    void setZListIndex(int z, bool saveUndoRedo = true);
    void updateChildrenId(int firstId, int lastId);
    void updateChildrenId(int firstId);
    void moveChildInList(int from, int to, bool saveUndoRedo = true);
    void removeChildFromList(int id, bool saveUndoRedo = true);
    void addChildToListAt(int index,
                          BoundingBox *child,
                          bool saveUndoRedo = true);

    virtual void selectAndAddContainedPointsToList
                            (QRectF absRect,QList<MovablePoint*> *list) {}

    QPointF getPivotAbsPos();
    bool isSelected();
    void select();
    void deselect();
    int getZIndex();
protected:
    virtual void updateAfterCombinedTransformationChanged();
    BoundingBoxType mType;
    QList<BoundingBox*> mChildren;
    BoundingBox *mParent = NULL;
    QMatrix mSavedTransformMatrix;
    QMatrix mTransformMatrix;
    QMatrix mCombinedTransformMatrix;
    int mZListIndex = 0;
    QPointF mAbsRotPivotPos;
    bool mPivotChanged = false;
    bool mSelected = false;
};


#endif // CHILDPARENT_H
