#ifndef CIRCLE_H
#define CIRCLE_H
#include "Boxes/pathbox.h"
#include "movablepoint.h"

class CircleCenterPoint : public PointAnimator
{
public:
    CircleCenterPoint(BoundingBox *parent, MovablePointType type);
    ~CircleCenterPoint();

    void setVerticalAndHorizontalPoints(MovablePoint *verticalPoint,
                                        MovablePoint *horizontalPoint);

    void moveByRel(const QPointF &relTranslatione);

    void startTransform();

    void finishTransform();
    void moveByAbs(const QPointF &absTranslatione);
private:
    MovablePoint *mVerticalPoint = NULL;
    MovablePoint *mHorizontalPoint = NULL;
};

class CircleRadiusPoint : public PointAnimator
{
public:
    CircleRadiusPoint(BoundingBox *parent,
                      const MovablePointType &type,
                      const bool &blockX,
                      MovablePoint *centerPoint);
    ~CircleRadiusPoint();

    void moveByRel(const QPointF &relTranslation);
//    void setAbsPosRadius(QPointF pos);
    void moveByAbs(const QPointF &absTranslatione);

    void startTransform();
    void finishTransform();
    void setRelativePos(const QPointF &relPos);
private:
    MovablePoint *mCenterPoint = NULL;
    bool mXBlocked = false;
};

class Circle : public PathBox
{
public:
    Circle();

    void setVerticalRadius(const qreal &verticalRadius);
    void setHorizontalRadius(const qreal &horizontalRadius);
    void setRadius(const qreal &radius);

    MovablePoint *getPointAtAbsPos(
                             const QPointF &absPtPos,
                             const CanvasMode &currentCanvasMode,
                             const qreal &canvasScaleInv);
    void selectAndAddContainedPointsToList(const QRectF &absRect,
                                           QList<MovablePoint *> *list);
    void moveRadiusesByAbs(const QPointF &absTrans);

    BoundingBox *createNewDuplicate() {
        return new Circle();
    }

    void drawSelectedSk(SkCanvas *canvas,
                        const CanvasMode &currentCanvasMode,
                        const SkScalar &invScale);

    void makeDuplicate(Property *targetBox) {
        PathBox::makeDuplicate(targetBox);
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

    bool SWT_isCircle() { return true; }

    void startAllPointsTransform();
    SkPath getPathAtRelFrame(const int &relFrame);
    void writeBoundingBox(QIODevice *target);
    void readBoundingBox(QIODevice *target);

    qreal getXRadiusAtRelFrame(const int &relFrame) {
        return mHorizontalRadiusPoint->getEffectiveXValueAtRelFrame(relFrame);
    }

    qreal getYRadiusAtRelFrame(const int &relFrame) {
        return mVerticalRadiusPoint->getEffectiveYValueAtRelFrame(relFrame);
    }

    qreal getCurrentXRadius() {
        return mHorizontalRadiusPoint->getEffectiveXValue();
    }

    qreal getCurrentYRadius() {
        return mVerticalRadiusPoint->getEffectiveYValue();
    }
protected:
    CircleCenterPoint *mCenter;
    CircleRadiusPoint *mHorizontalRadiusPoint;
    CircleRadiusPoint *mVerticalRadiusPoint;
};

#endif // CIRCLE_H
