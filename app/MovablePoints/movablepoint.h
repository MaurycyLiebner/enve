#ifndef MOVABLEPOINT_H
#define MOVABLEPOINT_H
#include <QPointF>
#include <QRectF>
#include <QPainter>
#include "Animators/qpointfanimator.h"
#include "skiaincludes.h"
#include "smartPointers/sharedpointerdefs.h"

class BoundingBox;
class NodePoint;
class BasicTransformAnimator;

class AnimatorUpdater;

enum MovablePointType : short {
    TYPE_PATH_POINT,
    TYPE_CTRL_POINT,
    TYPE_PIVOT_POINT,
    TYPE_GRADIENT_POINT,
    TYPE_BONE_POINT
};

class MovablePoint : public StdSelfRef {
    friend class StdSelfRef;
public:
    virtual ~MovablePoint() {}

    virtual void moveByRel(const QPointF &relTranslation) = 0;
    virtual void applyTransform(const QMatrix &transform) = 0;
    virtual void setRelativePos(const QPointF &relPos) = 0;
    virtual QPointF getRelativePos() const = 0;

    virtual void startTransform();
    virtual void finishTransform();
    virtual void cancelTransform() {}

    QPointF getAbsolutePos() const;

    virtual void drawSk(SkCanvas *canvas,
                        const SkScalar &invScale);

    bool isPointAtAbsPos(const QPointF &absPoint,
                         const qreal &canvasScaleInv);
    void setAbsolutePos(const QPointF &pos);

    BasicTransformAnimator *getParentTransform();

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
    bool isBonePoint();

    void rotateBy(const qreal &rot);
    void scale(const qreal &scaleBy);
    virtual void scale(const qreal &scaleXBy, const qreal &scaleYBy);
    virtual void saveTransformPivotAbsPos(const QPointF &absPivot);

    void setRadius(const qreal& radius);

    qreal getRadius();
    void moveToRel(const QPointF &relPos);
    virtual void scaleRelativeToSavedPivot(const qreal &sx,
                                           const qreal &sy);
    virtual void rotateRelativeToSavedPivot(const qreal &rot);


    void drawHovered(SkCanvas *canvas,
                     const SkScalar &invScale);

    QPointF mapRelativeToAbsolute(const QPointF &relPos) const;
    QPointF mapAbsoluteToRelative(const QPointF &absPos) const;

    void setRelativePosStartAndFinish(const QPointF &relPos) {
        //startTransform();
        setRelativePos(relPos);
        //finishTransform();
    }
    const QPointF &getSavedRelPos() const;

    void setParentTransformAnimator(
            BasicTransformAnimator *parentTransform) {
        mParentTransform_cv = parentTransform;
    }

    bool isSelected() const { return mSelected; }
protected:
    MovablePoint(BasicTransformAnimator* parentTransform,
                 const MovablePointType &type,
                 const qreal &radius = 7.5);

    bool mSelected = false;
    bool mTransformStarted = false;
    bool mHidden = false;
    MovablePointType mType;
    qreal mRadius;
    QPointF mSavedTransformPivot;
    QPointF mSavedRelPos;
    BasicTransformAnimator* mParentTransform_cv = nullptr;

    virtual void drawOnAbsPosSk(SkCanvas *canvas,
                const SkPoint &absPos,
                const SkScalar &invScale,
                const unsigned char &r,
                const unsigned char &g,
                const unsigned char &b,
                const bool &keyOnCurrent = false);
};

class NonAnimatedMovablePoint : public MovablePoint {
    friend class StdSelfRef;
public:
    void applyTransform(const QMatrix &transform){
        setRelativePosVal(transform.map(mCurrentPos));
    }

    void setRelativePos(const QPointF &relPos) {
        setRelativePosVal(relPos);
    }

    virtual void setRelativePosVal(const QPointF &relPos);

    QPointF getRelativePos() const {
        return mCurrentPos;
    }

    void moveByRel(const QPointF &relTranslation) {
        setRelativePos(mSavedRelPos + relTranslation);
    }

    void cancelTransform() {
        setRelativePos(mSavedRelPos);
    }
protected:
    NonAnimatedMovablePoint(BasicTransformAnimator* parentTransform,
                            const MovablePointType &type,
                            const qreal &radius = 7.5) :
        MovablePoint(parentTransform, type, radius) {

    }

    QPointF mCurrentPos;
};
#endif // MOVABLEPOINT_H
