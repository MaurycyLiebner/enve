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

    void moveByRel(QPointF absTranslatione);

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

    void moveByRel(QPointF relTranslation);
//    void setAbsPosRadius(QPointF pos);
    void moveByAbs(QPointF absTranslatione);

    void startTransform();
    void finishTransform();
    void setRelativePos(QPointF relPos, bool saveUndoRedo = true);
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

    void drawSelected(QPainter *p, const CanvasMode &currentCanvasMode);
    void updatePath();
    MovablePoint *getPointAt(const QPointF &absPtPos, const CanvasMode &currentCanvasMode);
    void selectAndAddContainedPointsToList(QRectF absRect, QList<MovablePoint *> *list);
    void moveRadiusesByAbs(QPointF absTrans);
    void startPointsTransform();

    int prp_saveToSql(QSqlQuery *query, const int &parentId);
    void prp_loadFromSql(const int &boundingBoxId);

    BoundingBox *createNewDuplicate(BoxesGroup *parent) {
        return new Circle(parent);
    }

    void prp_makeDuplicate(Property *targetBox) {
        PathBox::prp_makeDuplicate(targetBox);
        Circle *circleTarget = (Circle*)targetBox;
        circleTarget->duplicateCirclePointsFrom(
                    mCenter,
                    mHorizontalRadiusPoint,
                    mVerticalRadiusPoint);
    }
    void duplicateCirclePointsFrom(
                        CircleCenterPoint *center,
                        CircleRadiusPoint *horizontalRadiusPoint,
                        CircleRadiusPoint *verticalRadiusPoint);
private:
    CircleCenterPoint *mCenter;
    CircleRadiusPoint *mHorizontalRadiusPoint;
    CircleRadiusPoint *mVerticalRadiusPoint;
};

#endif // CIRCLE_H
