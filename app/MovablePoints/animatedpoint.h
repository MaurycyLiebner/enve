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
    ~AnimatedPoint() {}

    void startTransform();
    void finishTransform();

    QPointF getRelativePos() const;

    void moveByRel(const QPointF &relTranslation);

    virtual void removeApproximate() {}

    virtual void removeFromVectorPath() {}

    void setRelativePos(const QPointF &relPos);

    void cancelTransform();

    //virtual void prp_setUpdater(AnimatorUpdater *updater);

    virtual void applyTransform(const QMatrix &transform);

    virtual QPointF getRelativePosAtRelFrame(const int &frame) const;
protected:
    AnimatedPoint(
            QPointFAnimator* associatedAnimator,
            BasicTransformAnimator *parentTransform,
            const MovablePointType &type,
            const qreal &radius = 5.);

    QPointFAnimator* const mAssociatedAnimator_k;
};

#endif // ANIMATEDPOINT_H
