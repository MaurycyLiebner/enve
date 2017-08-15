#ifndef RECTANGLE_H
#define RECTANGLE_H
#include "Boxes/pathbox.h"

class RectangleTopLeftPoint : public MovablePoint {
public:
    RectangleTopLeftPoint(BoundingBox *parent);
};

class RectangleBottomRightPoint : public MovablePoint {
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
    void loadFromSql(const int &boundingBoxId);
    int saveToSql(QSqlQuery *query, const int &parentId);

    void makeDuplicate(Property *targetBox) {
        PathBox::makeDuplicate(targetBox);
        Rectangle *rectTarget = (Rectangle*)targetBox;
        rectTarget->duplicateRectanglePointsFrom(
                    mTopLeftPoint,
                    mBottomRightPoint,
                    &mXRadiusAnimator,
                    &mYRadiusAnimator);
    }
    void duplicateRectanglePointsFrom(RectangleTopLeftPoint *topLeftPoint,
                        RectangleBottomRightPoint *bottomRightPoint,
                        QrealAnimator *xRadiusAnimator,
                        QrealAnimator *yRadiusAnimator);
    BoundingBox *createNewDuplicate();
    MovablePoint *getBottomRightPoint();
    void finishAllPointsTransform();

    bool SWT_isRectangle() { return true; }
    SkPath getPathAtRelFrame(const int &relFrame);

    void setTopLeftPos(const QPointF &pos);
    void setBottomRightPos(const QPointF &pos);
    void setYRadius(const qreal &radiusY);
    void setXRadius(const qreal &radiusX);
protected:
    RectangleTopLeftPoint *mTopLeftPoint;
    RectangleBottomRightPoint *mBottomRightPoint;

    QrealAnimator mXRadiusAnimator;
    QrealAnimator mYRadiusAnimator;
};

#endif // RECTANGLE_H
