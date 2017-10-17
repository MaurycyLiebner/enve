#ifndef RECTANGLE_H
#define RECTANGLE_H
#include "Boxes/pathbox.h"

class RectangleTopLeftPoint : public PointAnimator {
public:
    RectangleTopLeftPoint(BoundingBox *parent);
};

class RectangleBottomRightPoint : public PointAnimator {
public:
    RectangleBottomRightPoint(BoundingBox *parent);
};

class Rectangle : public PathBox {
public:
    Rectangle();
    ~Rectangle();

    void moveSizePointByAbs(const QPointF &absTrans);
    void startAllPointsTransform();

    void drawSelectedSk(SkCanvas *canvas,
                        const CanvasMode &currentCanvasMode,
                        const SkScalar &invScale);

    MovablePoint *getPointAtAbsPos(const QPointF &absPtPos,
                             const CanvasMode &currentCanvasMode,
                             const qreal &canvasScaleInv);
    void selectAndAddContainedPointsToList(const QRectF &absRect,
                                           QList<MovablePoint *> *list);

    void makeDuplicate(Property *targetBox) {
        PathBox::makeDuplicate(targetBox);
        Rectangle *rectTarget = (Rectangle*)targetBox;
        rectTarget->duplicateRectanglePointsFrom(
                    mTopLeftPoint,
                    mBottomRightPoint,
                    mRadiusPoint);
    }
    void duplicateRectanglePointsFrom(RectangleTopLeftPoint *topLeftPoint,
                        RectangleBottomRightPoint *bottomRightPoint,
                        QPointFAnimator *radiusPoint);
    BoundingBox *createNewDuplicate();
    MovablePoint *getBottomRightPoint();
    void finishAllPointsTransform();

    bool SWT_isRectangle() { return true; }
    SkPath getPathAtRelFrame(const int &relFrame);

    void setTopLeftPos(const QPointF &pos);
    void setBottomRightPos(const QPointF &pos);
    void setYRadius(const qreal &radiusY);
    void setXRadius(const qreal &radiusX);
    void writeBoundingBox(QIODevice *target);
    void readBoundingBox(QIODevice *target);
protected:
    RectangleTopLeftPoint *mTopLeftPoint;
    RectangleBottomRightPoint *mBottomRightPoint;

    QPointFAnimator *mRadiusPoint;
};

#endif // RECTANGLE_H
