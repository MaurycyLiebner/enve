#ifndef GRADIENTPOINTS_H
#define GRADIENTPOINTS_H
#include "complexanimator.h"
#include "gradientpoint.h"

class MovablePoint;

class GradientPoints : public ComplexAnimator {
public:
    GradientPoints();
    ~GradientPoints();

    void initialize(PathBox *parentT, QPointF startPt = QPointF(0., 0.),
                    QPointF endPt = QPointF(100., 100.));

    void initialize(PathBox *parentT,
                    int fillGradientStartId, int fillGradientEndId);

    void clearAll();

    void enable();

    void disable();

    void drawGradientPoints(QPainter *p);

    MovablePoint *getPointAt(QPointF absPos);

    QPointF getStartPoint();

    QPointF getEndPoint();

    void setColors(QColor startColor, QColor endColor);

    bool enabled;
    GradientPoint *startPoint;
    GradientPoint *endPoint;
    PathBox *parent;

    void attachToBoneFromSqlZId();

    void setPositions(QPointF startPos, QPointF endPos, bool saveUndoRedo = true);
};

#endif // GRADIENTPOINTS_H
