#ifndef CIRCLE_H
#define CIRCLE_H
#include "MovablePoints/animatedpoint.h"

enum CanvasMode : short;

class CircleRadiusPoint : public AnimatedPoint {
    friend class StdSelfRef;
protected:
    CircleRadiusPoint(QPointFAnimator * const associatedAnimator,
                      BasicTransformAnimator * const parent,
                      AnimatedPoint * const centerPoint,
                      const MovablePointType &type,
                      const bool &blockX);
public:
    void setRelativePos(const QPointF &relPos);
private:
    const bool mXBlocked = false;
    const stdptr<AnimatedPoint> mCenterPoint;
};

#include "Boxes/pathbox.h"

class Circle : public PathBox {
    friend class SelfRef;
protected:
    Circle();
public:
    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                                   const CanvasMode &currentCanvasMode,
                                   const qreal &canvasScaleInv);
    void selectAndAddContainedPointsToList(const QRectF &absRect,
                                           QList<stdptr<MovablePoint>> &list);

    void drawCanvasControls(SkCanvas * const canvas,
                        const CanvasMode &currentCanvasMode,
                        const SkScalar &invScale);

    bool SWT_isCircle() const;

    void startAllPointsTransform();
    SkPath getPathAtRelFrameF(const qreal &relFrame);
    void writeBoundingBox(QIODevice *target);
    void readBoundingBox(QIODevice *target);

    bool differenceInEditPathBetweenFrames(
                const int& frame1, const int& frame2) const;

    void setVerticalRadius(const qreal &verticalRadius);
    void setHorizontalRadius(const qreal &horizontalRadius);
    void setRadius(const qreal &radius);
    void moveRadiusesByAbs(const QPointF &absTrans);

    qreal getCurrentXRadius();
    qreal getCurrentYRadius();
protected:
    void getMotionBlurProperties(QList<Property*> &list) const;
private:
    stdsptr<AnimatedPoint> mCenterPoint;
    stdsptr<CircleRadiusPoint> mHorizontalRadiusPoint;
    stdsptr<CircleRadiusPoint> mVerticalRadiusPoint;

    qsptr<QPointFAnimator> mCenterAnimator;
    qsptr<QPointFAnimator> mHorizontalRadiusAnimator;
    qsptr<QPointFAnimator> mVerticalRadiusAnimator;
};

#endif // CIRCLE_H
