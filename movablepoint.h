#ifndef MOVABLEPOINT_H
#define MOVABLEPOINT_H
#include <QPointF>
#include <QRectF>
#include <QPainter>
#include "SkPoint.h"
#include "Animators/qpointfanimator.h"
#include "transformable.h"

class BoundingBox;
class PathPoint;
class QSqlQuery;
class SkCanvas;

class AnimatorUpdater;

enum MovablePointType {
    TYPE_PATH_POINT,
    TYPE_CTRL_POINT,
    TYPE_PIVOT_POINT,
    TYPE_GRADIENT_POINT,
    TYPE_BONE_POINT
};

class MovablePoint :
    public QPointFAnimator,
    public Transformable {
public:
    MovablePoint(BoundingBox *parent,
                 const MovablePointType &type,
                 const qreal &radius = 7.5);

    virtual ~MovablePoint() {}

    virtual void startTransform();
    virtual void finishTransform();


    QPointF getRelativePos() const;

    QPointF getAbsolutePos() const;

    virtual void draw(QPainter *p);
    virtual void draw(SkCanvas *canvas,
                      const SkScalar &invScale);

    bool isPointAtAbsPos(const QPointF &absPoint,
                         const qreal &canvasScaleInv);
    void setAbsolutePos(const QPointF &pos,
                        const bool &saveUndoRedo = true);

    BoundingBox *getParent();

    bool isContainedInRect(const QRectF &absRect);
    virtual void moveByRel(const QPointF &relTranslation);
    virtual void moveToAbs(QPointF absPos);
    virtual void moveByAbs(const QPointF &absTranslatione);

    virtual void removeApproximate() {}

    void select();
    void deselect();

    virtual void removeFromVectorPath();

    void hide();
    void show();
    virtual bool isHidden();
    bool isVisible();
    void setVisible(bool bT);

    bool isPathPoint();
    bool isPivotPoint();
    bool isCtrlPoint();

    virtual void setRelativePos(const QPointF &relPos,
                                const bool &saveUndoRedo = true);
    void rotateBy(const qreal &rot);
    void scale(const qreal &scaleXBy, const qreal &scaleYBy);
    void saveTransformPivotAbsPos(const QPointF &absPivot);
    void scale(const qreal &scaleBy);
    void cancelTransform();

    void setRadius(qreal radius);

    bool isBeingTransformed();

    virtual void setPosAnimatorUpdater(AnimatorUpdater *updater);

    virtual void updateAfterFrameChanged(const int &frame);

    qreal getRadius();
    virtual void applyTransform(const QMatrix &transform);
    void moveToRel(const QPointF &relPos);
    void scaleRelativeToSavedPivot(const qreal &sx,
                                   const qreal &sy);
    virtual void rotateRelativeToSavedPivot(const qreal &rot);
    void removeAnimations();

    void makeDuplicate(MovablePoint *targetPoint);
    void duplicatePosAnimatorFrom(QPointFAnimator *source);

    virtual void drawHovered(QPainter *p);
    virtual void drawHovered(SkCanvas *canvas,
                             const SkScalar &invScale);

    QPointF mapRelativeToAbsolute(const QPointF &relPos) const;
    QPointF mapAbsoluteToRelative(const QPointF &absPos) const;
protected:
    bool mTransformStarted = false;
    MovablePointType mType;
    bool mHidden = false;
    qreal mRadius;
    QPointF mSavedRelPos;
    BoundingBox *mParent = NULL;
    virtual void drawOnAbsPos(QPainter *p,
                              const QPointF &absPos);
    virtual void drawOnAbsPos(SkCanvas *canvas,
                              const SkPoint &absPos,
                              const SkScalar &invScale,
                              const unsigned char r,
                              const unsigned char g,
                              const unsigned char b);
};

#endif // MOVABLEPOINT_H
