#ifndef CHILDPARENT_H
#define CHILDPARENT_H
#include <QMatrix>
#include "connectedtomainwindow.h"
#include "fillstrokesettings.h"
#include <QSqlQuery>

class UndoRedo;

class Canvas;

class UndoRedoStack;

class MovablePoint;

class PathPoint;

enum CanvasMode : short;

enum BoundingBoxType {
    TYPE_VECTOR_PATH,
    TYPE_GROUP,
    TYPE_CANVAS
};

class BoxesGroup;

class BoundingBox : public ConnectedToMainWindow
{
public:
    BoundingBox(BoxesGroup *parent, BoundingBoxType type);
    BoundingBox(MainWindow *window, BoundingBoxType type);
    BoundingBox(int boundingBoxId, BoxesGroup *parent,
                BoundingBoxType type);

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


    virtual bool pointInsidePath(QPointF) { return false; }
    virtual MovablePoint *getPointAt(QPointF, CanvasMode) { return NULL; }

    void moveUp();
    void moveDown();
    void bringToFront();
    void bringToEnd();

    void setZListIndex(int z, bool saveUndoRedo = true);

    virtual void selectAndAddContainedPointsToList
                            (QRectF,QList<MovablePoint*> *) {}

    QPointF getPivotAbsPos();
    bool isSelected();
    virtual void select();
    void deselect();
    int getZIndex();
    virtual void drawBoundingRect(QPainter *p);
    void setParent(BoxesGroup *parent, bool saveUndoRedo = true);
    BoxesGroup *getParent();

    bool isGroup();
    virtual BoundingBox *getPathAtFromAllAncestors(QPointF absPos);

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
    void setPivotRelPos(QPointF relPos, bool saveUndoRedo = true, bool pivotChanged = true);

    void scheduleRemoval();
    void descheduleRemoval();
    bool isScheduldedForRemoval();

    void scaleCenter(qreal scaleXBy, qreal scaleYBy);
    void scaleCenter(qreal scaleBy);
    void scaleRight(qreal scaleXBy);
    void scaleLeft(qreal scaleXBy);
    void scaleTop(qreal scaleYBy);
    void scaleBottom(qreal scaleYBy);
    void scaleBottomRight(qreal scaleXBy, qreal scaleYBy);
    void scaleBottomLeft(qreal scaleXBy, qreal scaleYBy);
    void scaleTopRight(qreal scaleXBy, qreal scaleYBy);
    void scaleTopLeft(qreal scaleXBy, qreal scaleYBy);
    void scaleRightFixedRatio(qreal scaleXBy);
    void scaleBottomFixedRatio(qreal scaleYBy);
    void scaleTopFixedRatio(qreal scaleYBy);
    void scaleLeftFixedRatio(qreal scaleXBy);
    void scaleBottomRight(qreal scaleBy);
    void scaleBottomLeft(qreal scaleBy);
    void scaleTopRight(qreal scaleBy);
    void scaleTopLeft(qreal scaleBy);
    void cancelTransform();
    void scaleFromSaved(qreal scaleXBy, qreal scaleYBy);

    virtual int saveToSql(int parentId);

    virtual void clearAll() {}

    virtual PathPoint *createNewPointOnLineNear(QPointF) { return NULL; }
    bool isPath();
    void saveTransformPivot(QPointF absPivot);
    void scaleFromSaved(qreal scaleXBy, qreal scaleYBy, QPointF relOrigin);

    //

    virtual void drawListItem(QPainter *p, qreal drawX, qreal drawY, qreal maxY);
    virtual qreal getListItemHeight();
    void showChildrenListItems();
    void hideChildrenListItems();
    void setName(QString name);
    QString getName();

    void hide();
    void show();
    bool isVisible();
    void setVisibile(bool visible, bool saveUndoRedo = true);
    virtual void handleListItemMousePress(qreal relX, qreal relY);
    void setChildrenListItemsVisible(bool bt);
    void lock();
    void unlock();
    void setLocked(bool bt);
    bool isLocked();
    bool isVisibleAndUnlocked();
protected:
    virtual void updateAfterCombinedTransformationChanged() {}

    bool mScheduledForRemove = false;
    BoundingBoxType mType;
    BoxesGroup *mParent = NULL;

    QPointF mSavedTransformPivot;
    QMatrix mSavedTransformMatrix;

    QMatrix mTransformMatrix;

    QMatrix mCombinedTransformMatrix;
    int mZListIndex = 0;
    QPointF mRelRotPivotPos;
    bool mPivotChanged = false;
    bool mSelected = false;

    bool mVisible = true;
    bool mLocked = false;
    //

    bool mChildrenListItemsVisibile = false;
    QString mName = "";
};


#endif // CHILDPARENT_H
