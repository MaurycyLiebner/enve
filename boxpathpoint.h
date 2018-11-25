#ifndef BOXPATHPOINT_H
#define BOXPATHPOINT_H
#include "pointanimator.h"
class BoundingBox;
class BoxTransformAnimator;
class BoxPathPoint : public PointAnimatorMovablePoint {
    friend class StdSelfRef;
public:
    BoxPathPoint(QPointFAnimator *associatedAnimator,
                 BoxTransformAnimator *box);

    void startTransform();
    void finishTransform();
    void moveByAbs(const QPointF &absTranslatione);
    void drawSk(SkCanvas *canvas, const SkScalar &invScale);
private:
    QPointF mSavedAbsPos;
};

#endif // BOXPATHPOINT_H
