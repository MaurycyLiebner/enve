#ifndef CIRCLE_H
#define CIRCLE_H
#include "Boxes/pathbox.h"
#include "movablepoint.h"

class CircleCenterPoint : public PointAnimatorMovablePoint {
    friend class StdSelfRef;
public:
    void setVerticalAndHorizontalPoints(MovablePoint *verticalPoint,
                                        MovablePoint* horizontalPoint);

    void moveByRel(const QPointF &relTranslatione);

    void startTransform();

    void finishTransform();
    void moveByAbs(const QPointF &absTranslatione);
protected:
    CircleCenterPoint(QPointFAnimator *associatedAnimator,
                      BasicTransformAnimator *parent,
                      const MovablePointType &type);
private:
    stdptr<MovablePoint>mVerticalPoint_cv;
    stdptr<MovablePoint>mHorizontalPoint_cv;
};

class CircleRadiusPoint : public PointAnimatorMovablePoint {
    friend class StdSelfRef;
public:
    void moveByRel(const QPointF &relTranslation);
//    void setAbsPosRadius(QPointF pos);
    void moveByAbs(const QPointF &absTranslatione);

    void startTransform();
    void finishTransform();
    void setRelativePos(const QPointF &relPos);
protected:
    CircleRadiusPoint(QPointFAnimator *associatedAnimator,
                      BasicTransformAnimator *parent,
                      const MovablePointType &type,
                      const bool &blockX,
                      MovablePoint *centerPoint);
private:
    bool mXBlocked = false;
    stdptr<MovablePoint>mCenterPoint_cv;
};

class Circle : public PathBox {
    friend class SelfRef;
public:
    void setVerticalRadius(const qreal &verticalRadius);
    void setHorizontalRadius(const qreal &horizontalRadius);
    void setRadius(const qreal &radius);

    MovablePoint *getPointAtAbsPos(
                             const QPointF &absPtPos,
                             const CanvasMode &currentCanvasMode,
                             const qreal &canvasScaleInv);
    void selectAndAddContainedPointsToList(const QRectF &absRect,
                                           QList<stdptr<MovablePoint>> &list);
    void moveRadiusesByAbs(const QPointF &absTrans);

    void drawSelectedSk(SkCanvas *canvas,
                        const CanvasMode &currentCanvasMode,
                        const SkScalar &invScale);

    bool SWT_isCircle() { return true; }

    void startAllPointsTransform();
    SkPath getPathAtRelFrame(const int &relFrame);
    SkPath getPathAtRelFrameF(const qreal &relFrame);
    void writeBoundingBox(QIODevice *target);
    void readBoundingBox(QIODevice *target);

    qreal getXRadiusAtRelFrame(const int &relFrame) {
        return mHorizontalRadiusAnimator->getEffectiveXValueAtRelFrame(relFrame);
    }

    qreal getYRadiusAtRelFrame(const int &relFrame) {
        return mVerticalRadiusAnimator->getEffectiveYValueAtRelFrame(relFrame);
    }

    qreal getCurrentXRadius() {
        return mHorizontalRadiusAnimator->getEffectiveXValue();
    }

    qreal getCurrentYRadius() {
        return mVerticalRadiusAnimator->getEffectiveYValue();
    }

    bool differenceInEditPathBetweenFrames(
                const int& frame1, const int& frame2) const;
protected:
    Circle();

    void getMotionBlurProperties(QList<Property*>& list);

    stdsptr<CircleCenterPoint> mCenterPoint;
    stdsptr<CircleRadiusPoint> mHorizontalRadiusPoint;
    stdsptr<CircleRadiusPoint> mVerticalRadiusPoint;

    qsptr<QPointFAnimator> mCenterAnimator;
    qsptr<QPointFAnimator> mHorizontalRadiusAnimator;
    qsptr<QPointFAnimator> mVerticalRadiusAnimator;
};

#endif // CIRCLE_H
