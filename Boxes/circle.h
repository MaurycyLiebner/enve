#ifndef CIRCLE_H
#define CIRCLE_H
#include "Boxes/pathbox.h"
#include "movablepoint.h"

class CircleCenterPoint : public MovablePoint
{
public:
    CircleCenterPoint(BoundingBox *parent, MovablePointType type);
    ~CircleCenterPoint();

    void setVerticalAndHorizontalPoints(MovablePoint *verticalPoint,
                                        MovablePoint *horizontalPoint);

    void moveBy(QPointF absTranslatione);

    void startTransform();

    void finishTransform();
    void moveByAbs(QPointF absTranslatione);
private:
    MovablePoint *mVerticalPoint = NULL;
    MovablePoint *mHorizontalPoint = NULL;
};

class CircleRadiusPoint : public MovablePoint
{
public:
    CircleRadiusPoint(BoundingBox *parent, MovablePointType type,
                      bool blockX, MovablePoint *centerPoint);
    ~CircleRadiusPoint();

    void moveBy(QPointF relTranslation);
//    void setAbsPosRadius(QPointF pos);
    void moveByAbs(QPointF absTranslatione);

    void startTransform();
    void finishTransform();
private:
    MovablePoint *mCenterPoint = NULL;
    bool mXBlocked = false;
};

class Circle : public PathBox
{
public:
    Circle(BoxesGroup *parent);

    void setVerticalRadius(qreal verticalRadius);
    void setHorizontalRadius(qreal horizontalRadius);
    void setRadius(qreal radius);

    void drawSelected(QPainter *p, CanvasMode currentCanvasMode);
    void updatePath();
    MovablePoint *getPointAt(QPointF absPtPos, CanvasMode currentCanvasMode);
    void selectAndAddContainedPointsToList(QRectF absRect, QList<MovablePoint *> *list);
    void moveRadiusesByAbs(QPointF absTrans);
    void startPointsTransform();
    void updateAfterFrameChanged(int currentFrame);

    int saveToSql(QSqlQuery *query, int parentId);
    void loadFromSql(int boundingBoxId);
private:
    CircleCenterPoint *mCenter;
    CircleRadiusPoint *mHorizontalRadiusPoint;
    CircleRadiusPoint *mVerticalRadiusPoint;
};

#endif // CIRCLE_H
