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
protected:
    AnimatedPoint(QPointFAnimator * const associatedAnimator,
                  const MovablePointType &type);
    AnimatedPoint(QPointFAnimator * const associatedAnimator,
                  BasicTransformAnimator * const trans,
                  const MovablePointType &type);
public:    
    QPointF getRelativePos() const;
    void setRelativePos(const QPointF &relPos);

    void startTransform();
    void finishTransform();
    void cancelTransform();

    QPointFAnimator * getAnimator() const {
        return mAssociatedAnimator_k;
    }

    QrealAnimator * getXAnimator() const {
        return mAssociatedAnimator_k->getXAnimator();
    }

    QrealAnimator * getYAnimator() const {
        return mAssociatedAnimator_k->getYAnimator();
    }
protected:
    void setValue(const QPointF& value) {
        mAssociatedAnimator_k->setBaseValue(value);
    }

    QPointF getValue() const {
        return mAssociatedAnimator_k->getBaseValue();
    }
private:
    QPointFAnimator* const mAssociatedAnimator_k;
};

#endif // ANIMATEDPOINT_H
