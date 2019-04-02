#ifndef ANIMATEDPOINT_H
#define ANIMATEDPOINT_H

#include "movablepoint.h"
#include <QPointF>
#include <QRectF>
#include <QPainter>
#include "Animators/qpointfanimator.h"
#include "skia/skiaincludes.h"

class BoundingBox;
class NodePoint;

class PropertyUpdater;

class QPointFAnimator;

class AnimatedPoint : public MovablePoint {
    friend class StdSelfRef;
public:    
    QPointF getRelativePos() const final;
    void setRelativePos(const QPointF &relPos);

    void startTransform();
    void finishTransform();
    void cancelTransform();
protected:
    AnimatedPoint(QPointFAnimator* associatedAnimator,
                  BasicTransformAnimator *parentTransform,
                  const MovablePointType &type,
                  const qreal &radius = 5);

    QPointFAnimator* const mAssociatedAnimator_k;
};

#endif // ANIMATEDPOINT_H
