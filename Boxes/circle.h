#ifndef CIRCLE_H
#define CIRCLE_H
#include "Boxes/pathbox.h"
#include "movablepoint.h"

class CircleCenterPoint : public PointAnimator
{
public:
    CircleCenterPoint(BasicTransformAnimator *parent, MovablePointType type);
    ~CircleCenterPoint();

    void setVerticalAndHorizontalPoints(MovablePoint *verticalPoint,
                                        MovablePoint *horizontalPoint);

    void moveByRel(const QPointF &relTranslatione);

    void startTransform();

    void finishTransform();
    void moveByAbs(const QPointF &absTranslatione);
private:
    MovablePoint *mVerticalPoint = nullptr;
    MovablePoint *mHorizontalPoint = nullptr;
};

class CircleRadiusPoint : public PointAnimator
{
public:
    CircleRadiusPoint(BasicTransformAnimator *parent,
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
    MovablePoint *mCenterPoint = nullptr;
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

    void drawSelectedSk(SkCanvas *canvas,
                        const CanvasMode &currentCanvasMode,
                        const SkScalar &invScale);

    bool SWT_isCircle() { return true; }

    void startAllPointsTransform();
    SkPath getPathAtRelFrame(const int &relFrame);
    SkPath getPathAtRelFrameF(const qreal &relFrame);
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
    void getMotionBlurProperties(QList<Property *> *list);
};

#endif // CIRCLE_H
