#ifndef GRADIENTPOINTS_H
#define GRADIENTPOINTS_H
#include "Animators/complexanimator.h"
class GradientPoint;
class MovablePoint;
class PathBox;
#include "skiaincludes.h"

class GradientPoints : public ComplexAnimator {
public:
    GradientPoints();
    ~GradientPoints();

    void initialize(PathBox *parentT);

    void enable();

    void disable();

    void drawGradientPointsSk(SkCanvas *canvas,
                              const SkScalar &invScale);

    MovablePoint *qra_getPointAt(const QPointF &absPos,
                                 const qreal &canvasScaleInv);

    void setColors(QColor startColor, QColor endColor);

    bool enabled;
    GradientPoint *startPoint;
    GradientPoint *endPoint;
    PathBox *parent;

    void setPositions(const QPointF &startPos,
                      const QPointF &endPos);

    Property *makeDuplicate() {
        return NULL;
    }
    void makeDuplicate(Property *target);
    void duplicatePointsFrom(GradientPoint *startPointT,
                             GradientPoint *endPointT);
    QPointF getStartPointAtRelFrame(const int &relFrame);
    QPointF getEndPointAtRelFrame(const int &relFrame);
    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);
};

#endif // GRADIENTPOINTS_H
