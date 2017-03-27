#ifndef RECTANGLE_H
#define RECTANGLE_H
#include "Boxes/pathbox.h"

class RectangleTopLeftPoint : public MovablePoint
{
public:
    RectangleTopLeftPoint(BoundingBox *parent);
};

class RectangleBottomRightPoint : public MovablePoint
{
public:
    RectangleBottomRightPoint(BoundingBox *parent);
};

class Rectangle : public PathBox
{
public:
    Rectangle(BoxesGroup *parent);
    ~Rectangle();

    void moveSizePointByAbs(QPointF absTrans);
    void startAllPointsTransform();
    void drawSelected(QPainter *p,
                      const CanvasMode &currentCanvasMode);
    MovablePoint *getPointAt(const QPointF &absPtPos,
                             const CanvasMode &currentCanvasMode);
    void selectAndAddContainedPointsToList(QRectF absRect,
                                           QList<MovablePoint *> *list);
    void updatePath();
    void prp_loadFromSql(const int &boundingBoxId);
    int prp_saveToSql(QSqlQuery *query, const int &parentId);

    void makeDuplicate(BoundingBox *targetBox) {
        PathBox::makeDuplicate(targetBox);
        Rectangle *rectTarget = (Rectangle*)targetBox;
        rectTarget->duplicateRectanglePointsFrom(
                    mTopLeftPoint,
                    mBottomRightPoint,
                    &mRadiusAnimator);
    }
    void duplicateRectanglePointsFrom(RectangleTopLeftPoint *topLeftPoint,
                        RectangleBottomRightPoint *bottomRightPoint,
                        QrealAnimator *radiusAnimator);
    BoundingBox *createNewDuplicate(BoxesGroup *parent);
    MovablePoint *getBottomRightPoint();
    void finishAllPointsTransform();
private:
    RectangleTopLeftPoint *mTopLeftPoint;
    RectangleBottomRightPoint *mBottomRightPoint;

    QrealAnimator mRadiusAnimator;
};

#endif // RECTANGLE_H
