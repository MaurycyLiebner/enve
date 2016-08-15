#ifndef CHILDPARENT_H
#define CHILDPARENT_H
#include <QMatrix>
#include "connectedtomainwindow.h"
#include "fillstrokesettings.h"

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
    virtual bool isContainedIn(QRectF absRect);
    virtual QRectF getBoundingRect() { return QRectF(); }

    virtual void draw(QPainter *) {}
    virtual void drawSelected(QPainter *, CanvasMode) {}

    QMatrix getCombinedTransform();

    QPointF getTranslation();

    void scale(qreal scaleBy, QPointF absOrigin);
    void scale(qreal scaleXBy, qreal scaleYBy, QPointF absOrigin);

    void rotateBy(qreal rot, QPointF absOrigin);

    void setTransformation(QMatrix transMatrix);

    QPointF getAbsolutePos();

    void updateCombinedTransform();
    void moveBy(QPointF trans);

    void startTransform();
    void finishTransform();

    virtual void addChild(BoundingBox *child);
    virtual void removeChild(BoundingBox *child);

    virtual bool pointInsidePath(QPointF) { return false; }
    virtual MovablePoint *getPointAt(QPointF, CanvasMode) { return NULL; }

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
    virtual void removeChildFromList(int id, bool saveUndoRedo = true);
    virtual void addChildToListAt(int index,
                          BoundingBox *child,
                          bool saveUndoRedo = true);

    virtual void selectAndAddContainedPointsToList
                            (QRectF,QList<MovablePoint*> *) {}

    QPointF getPivotAbsPos();
    bool isSelected();
    virtual void select();
    void deselect();
    int getZIndex();
    virtual void drawBoundingRect(QPainter *p);
    void setParent(BoundingBox *parent, bool saveUndoRedo = true);
    BoundingBox *getParent();

    bool isGroup();
    virtual BoundingBox *getBoxAtFromAllAncestors(QPointF absPos);

    virtual void setStrokeSettings(StrokeSettings, bool saveUndoRedo = true) { Q_UNUSED(saveUndoRedo) }
    virtual void setFillSettings(PaintSettings, bool saveUndoRedo = true) { Q_UNUSED(saveUndoRedo) }

    virtual PaintSettings getFillSettings();
    virtual StrokeSettings getStrokeSettings();

    virtual qreal getCurrentCanvasScale();

    virtual void startStrokeTransform() {}
    virtual void startFillTransform() {}
    virtual void finishStrokeTransform() {}
    virtual void finishFillTransform() {}
    void setPivotAbsPos(QPointF absPos, bool saveUndoRedo = true, bool pivotChanged = true);
    void applyTransformation(QMatrix transformation);
protected:
    virtual void updateAfterCombinedTransformationChanged();
    BoundingBoxType mType;
    QList<BoundingBox*> mChildren;
    BoundingBox *mParent = NULL;

    QPointF mSavedTransformAbsPos;
    QMatrix mSavedTransformMatrix;
    QMatrix mTransformMatrix;

    QMatrix mCombinedTransformMatrix;
    int mZListIndex = 0;
    QPointF mRelRotPivotPos;
    bool mPivotChanged = false;
    bool mSelected = false;
};


#endif // CHILDPARENT_H
