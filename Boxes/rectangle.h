#ifndef RECTANGLE_H
#define RECTANGLE_H
#include "Boxes/pathbox.h"

class RectangleTopLeftPoint : public MovablePoint
{
public:
    RectangleTopLeftPoint(BoundingBox *parent);

    void moveByRel(QPointF absTranslatione);
    void moveByAbs(QPointF absTranslatione);
    void startTransform();
    void finishTransform();

    void setBottomRightPoint(MovablePoint *bottomRightPoint);
private:
    MovablePoint *mBottomRightPoint;
};

class RectangleBottomRightPoint : public MovablePoint
{
public:
    RectangleBottomRightPoint(BoundingBox *parent);
    void setPoints(MovablePoint *topLeftPoint, MovablePoint *radiusPoint);

    void moveByRel(QPointF absTranslatione);
    void moveByAbs(QPointF absTranslatione);
    void startTransform();
    void finishTransform();
private:
    MovablePoint *mTopLeftPoint;
    MovablePoint *mRadiusPoint;
};

class RectangleRadiusPoint : public MovablePoint
{
public:
    RectangleRadiusPoint(BoundingBox *parent);
    void setPoints(MovablePoint *topLeftPoint, MovablePoint *bottomRightPoint);

    void moveByRel(QPointF absTranslatione);
    void moveByAbs(QPointF absTranslatione);
    void startTransform();
    void finishTransform();
    void setAbsPosRadius(QPointF pos);
private:
    MovablePoint *mTopLeftPoint;
    MovablePoint *mBottomRightPoint;
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
    MovablePoint *getPointAt(const QPointF &absPtPos, const CanvasMode &currentCanvasMode);
    void selectAndAddContainedPointsToList(QRectF absRect,
                                           QList<MovablePoint *> *list);
    void updatePath();
    void updateRadiusXAndRange();
    void updateAfterFrameChanged(int currentFrame);
    void loadFromSql(int boundingBoxId);
    int saveToSql(QSqlQuery *query, int parentId);

    void makeDuplicate(BoundingBox *targetBox) {
        PathBox::makeDuplicate(targetBox);
        Rectangle *rectTarget = (Rectangle*)targetBox;
        rectTarget->duplicateRectanglePointsFrom(
                    mTopLeftPoint,
                    mBottomRightPoint,
                    mRadiusPoint);
    }
    void duplicateRectanglePointsFrom(
                        RectangleTopLeftPoint *topLeftPoint,
                        RectangleBottomRightPoint *bottomRightPoint,
                        RectangleRadiusPoint *radiusPoint);
    BoundingBox *createNewDuplicate(BoxesGroup *parent);
private:
    RectangleTopLeftPoint *mTopLeftPoint;
    RectangleBottomRightPoint *mBottomRightPoint;

    RectangleRadiusPoint *mRadiusPoint;

    QrealAnimator *mRadiusAnimator;
};

#endif // RECTANGLE_H
