#ifndef MOVABLEPOINT_H
#define MOVABLEPOINT_H
#include <QPointF>
#include <QRectF>
#include <QPainter>
#include "Animators/qpointfanimator.h"
#include "transformable.h"
#include "skiaincludes.h"

class BoundingBox;
class NodePoint;
class QSqlQuery;

class AnimatorUpdater;

enum MovablePointType {
    TYPE_PATH_POINT,
    TYPE_CTRL_POINT,
    TYPE_PIVOT_POINT,
    TYPE_GRADIENT_POINT,
    TYPE_BONE_POINT
};

class MovablePoint :
    public Transformable {
public:
    MovablePoint(BoundingBox *parent,
                 const MovablePointType &type,
                 const qreal &radius = 7.5);

    virtual ~MovablePoint() {}

    virtual void startTransform();
    virtual void finishTransform();

    QPointF getAbsolutePos() const;

    virtual void drawSk(SkCanvas *canvas,
                        const SkScalar &invScale);

    bool isPointAtAbsPos(const QPointF &absPoint,
                         const qreal &canvasScaleInv);
    void setAbsolutePos(const QPointF &pos);

    BoundingBox *getParent();

    bool isContainedInRect(const QRectF &absRect);
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
    void setVisible(const bool &bT);

    bool isNodePoint();
    bool isPivotPoint();
    bool isCtrlPoint();

    void rotateBy(const qreal &rot);
    void scale(const qreal &scaleXBy, const qreal &scaleYBy);
    void saveTransformPivotAbsPos(const QPointF &absPivot);
    void scale(const qreal &scaleBy);
    void cancelTransform();

    void setRadius(qreal radius);

    bool isBeingTransformed();

    qreal getRadius();
    void moveToRel(const QPointF &relPos);
    void scaleRelativeToSavedPivot(const qreal &sx,
                                   const qreal &sy);
    virtual void rotateRelativeToSavedPivot(const qreal &rot);


    virtual void drawHovered(SkCanvas *canvas,
                             const qreal &invScale);

    QPointF mapRelativeToAbsolute(const QPointF &relPos) const;
    QPointF mapAbsoluteToRelative(const QPointF &absPos) const;

    virtual void applyTransform(const QMatrix &transform) = 0;
    virtual void setRelativePos(const QPointF &relPos) = 0;
    virtual QPointF getRelativePos() const = 0;
    virtual void moveByRel(const QPointF &relTranslation) = 0;
protected:
    bool mTransformStarted = false;
    MovablePointType mType;
    bool mHidden = false;
    qreal mRadius;
    QPointF mSavedRelPos;
    BoundingBox *mParent = NULL;

    virtual void drawOnAbsPosSk(SkCanvas *canvas,
                const SkPoint &absPos,
                const SkScalar &invScale,
                const unsigned char r,
                const unsigned char g,
                const unsigned char b);
};

class NonAnimatedMovablePoint : public MovablePoint {
public:
    NonAnimatedMovablePoint(BoundingBox *parent,
                            const MovablePointType &type,
                            const qreal &radius = 7.5) :
        MovablePoint(parent, type, radius) {

    }

    void applyTransform(const QMatrix &transform){
        mCurrentPos = transform.map(mCurrentPos);
    }

    void setRelativePos(const QPointF &relPos) {
        mCurrentPos = relPos;
    }

    QPointF getRelativePos() const {
        return mCurrentPos;
    }

    void moveByRel(const QPointF &relTranslation) {
        mCurrentPos = mSavedRelPos + relTranslation;
    }
protected:
    QPointF mCurrentPos;
};
#endif // MOVABLEPOINT_H
