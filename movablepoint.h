#ifndef MOVABLEPOINT_H
#define MOVABLEPOINT_H
#include <QPointF>
#include <QRectF>
#include <QPainter>
#include "qpointfanimator.h"
#include "transformable.h"

class BoundingBox;
class PathPoint;

class AnimatorUpdater;

enum MovablePointType {
    TYPE_PATH_POINT,
    TYPE_CTRL_POINT,
    TYPE_PIVOT_POINT,
    TYPE_GRADIENT_POINT
};

class MovablePoint : public Transformable
{
public:
    MovablePoint(qreal relPosX, qreal relPosY, BoundingBox *parent,
                 MovablePointType type, qreal radius = 7.5f);
    MovablePoint(QPointF absPos, BoundingBox *parent, MovablePointType type,
                 qreal radius = 7.5f);
    MovablePoint(int movablePointId, BoundingBox *parent,
                 MovablePointType type, qreal radius = 7.5f);

    virtual void startTransform();
    virtual void finishTransform();


    QPointF getRelativePos();

    QPointF getAbsolutePos();

    virtual void draw(QPainter *p);

    bool isPointAt(QPointF absPoint);
    void setAbsolutePos(QPointF pos, bool saveUndoRedo = true);

    BoundingBox *getParent();

    bool isContainedInRect(QRectF absRect);
    virtual void moveBy(QPointF relTranslation);
    virtual void moveToAbs(QPointF absPos);
    virtual void moveByAbs(QPointF absTranslatione);

    virtual void removeApproximate() {}

    void select();
    void deselect();

    virtual void remove();

    void hide();
    void show();
    virtual bool isHidden();
    bool isVisible();
    void setVisible(bool bT);

    bool isPathPoint();
    bool isPivotPoint();
    bool isCtrlPoint();

    virtual void setRelativePos(QPointF relPos, bool saveUndoRedo = true);
    void rotateBy(qreal rot);
    void scale(qreal scaleXBy, qreal scaleYBy);
    void saveTransformPivot(QPointF absPivot);
    void scale(qreal scaleBy);
    void cancelTransform();

    void setRadius(qreal radius);

    QPointF getAbsBoneAttachPoint();

    void attachToBoneFromSqlZId();

    bool isBeingTransformed();
    virtual int saveToSql();

    virtual void setPosAnimatorUpdater(AnimatorUpdater *updater);

    virtual void updateAfterFrameChanged(int frame);

    QPointFAnimator *getRelativePosAnimatorPtr();

    qreal getRadius();
protected:
    bool mTransformStarted = false;
    MovablePointType mType;
    bool mHidden = false;
    qreal mRadius;
    QPointFAnimator mRelPos;
    QPointF mSavedRelPos;
    BoundingBox *mParent = NULL;
};

#endif // MOVABLEPOINT_H
