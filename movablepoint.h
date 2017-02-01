#ifndef MOVABLEPOINT_H
#define MOVABLEPOINT_H
#include <QPointF>
#include <QRectF>
#include <QPainter>
#include "Animators/qpointfanimator.h"
#include "transformable.h"

class BoundingBox;
class PathPoint;
class QSqlQuery;

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
    MovablePoint(BoundingBox *parent, MovablePointType type,
                 qreal radius = 7.5);

    virtual void startTransform();
    virtual void finishTransform();


    QPointF getRelativePos() const;

    QPointF getAbsolutePos() const;

    virtual void draw(QPainter *p);

    bool isPointAtAbsPos(QPointF absPoint);
    void setAbsolutePos(QPointF pos,
                        bool saveUndoRedo = true);

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
    void saveTransformPivotAbsPos(QPointF absPivot);
    void scale(qreal scaleBy);
    void cancelTransform();

    void setRadius(qreal radius);

    bool isBeingTransformed();
    virtual int saveToSql(QSqlQuery *query);

    virtual void setPosAnimatorUpdater(AnimatorUpdater *updater);

    virtual void updateAfterFrameChanged(int frame);

    QPointFAnimator *getRelativePosAnimatorPtr();

    qreal getRadius();
    void loadFromSql(int movablePointId);
    QPointF mapRelativeToAbsolute(QPointF relPos) const;
    virtual void applyTransform(QMatrix transform);
    void moveToRel(QPointF relPos);
    void scaleRelativeToSavedPivot(qreal sx, qreal sy);
    void rotateRelativeToSavedPivot(qreal rot);
    void removeAnimations();
protected:
    bool mTransformStarted = false;
    MovablePointType mType;
    bool mHidden = false;
    qreal mRadius;
    QPointFAnimator mRelPos;
    QPointF mSavedRelPos;
    BoundingBox *mParent = NULL;
    void drawOnAbsPos(QPainter *p, const QPointF &absPos);
};

#endif // MOVABLEPOINT_H
