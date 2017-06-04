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

    QPointF getStartPoint();

    QPointF getEndPoint();

    void setColors(QColor startColor, QColor endColor);

    bool enabled;
    GradientPoint *startPoint;
    GradientPoint *endPoint;
    PathBox *parent;

    void setPositions(QPointF startPos,
                      QPointF endPos,
                      bool saveUndoRedo = true);
    void prp_loadFromSql(const int &identifyingId);
    int prp_saveToSql(QSqlQuery *query, const int &parentId = 0);
    Property *makeDuplicate() {
        return NULL;
    }
    void makeDuplicate(Property *target);
    void duplicatePointsFrom(GradientPoint *startPointT,
                             GradientPoint *endPointT);
};

#endif // GRADIENTPOINTS_H
