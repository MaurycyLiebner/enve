#ifndef RECTANGLE_H
#define RECTANGLE_H
#include "Boxes/pathbox.h"
class AnimatedPoint;
class QPointFAnimator;
class Rectangle : public PathBox {
    friend class SelfRef;
public:
    void moveSizePointByAbs(const QPointF &absTrans);
    void startAllPointsTransform();

    void drawSelectedSk(SkCanvas *canvas,
                        const CanvasMode &currentCanvasMode,
                        const SkScalar &invScale);

    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                             const CanvasMode &currentCanvasMode,
                             const qreal &canvasScaleInv);
    void selectAndAddContainedPointsToList(const QRectF &absRect,
                                           QList<stdptr<MovablePoint>> &list);

    MovablePoint *getBottomRightPoint();
    void finishAllPointsTransform();

    bool SWT_isRectangle() const { return true; }
    SkPath getPathAtRelFrame(const int &relFrame);
    SkPath getPathAtRelFrameF(const qreal &relFrame);

    void setTopLeftPos(const QPointF &pos);
    void setBottomRightPos(const QPointF &pos);
    void setYRadius(const qreal &radiusY);
    void setXRadius(const qreal &radiusX);
    void writeBoundingBox(QIODevice *target);
    void readBoundingBox(QIODevice *target);
    bool differenceInEditPathBetweenFrames(
                const int& frame1, const int& frame2) const;
protected:
    Rectangle();

    qsptr<QPointFAnimator> mTopLeftAnimator;
    qsptr<QPointFAnimator> mBottomRightAnimator;
    qsptr<QPointFAnimator> mRadiusAnimator;

    stdsptr<AnimatedPoint> mTopLeftPoint;
    stdsptr<AnimatedPoint> mBottomRightPoint;
    stdsptr<AnimatedPoint> mRadiusPoint;

    void getMotionBlurProperties(QList<Property*> &list);
};

#endif // RECTANGLE_H
