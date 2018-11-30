#ifndef RECTANGLE_H
#define RECTANGLE_H
#include "Boxes/pathbox.h"

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
                                           QList<MovablePointPtr> &list);

    MovablePoint *getBottomRightPoint();
    void finishAllPointsTransform();

    bool SWT_isRectangle() { return true; }
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

    QPointFAnimatorQSPtr mTopLeftAnimator;
    QPointFAnimatorQSPtr mBottomRightAnimator;
    QPointFAnimatorQSPtr mRadiusAnimator;

    PointAnimatorMovablePointSPtr mTopLeftPoint;
    PointAnimatorMovablePointSPtr mBottomRightPoint;
    PointAnimatorMovablePointSPtr mRadiusPoint;

    void getMotionBlurProperties(QList<Property*> &list);
};

#endif // RECTANGLE_H
