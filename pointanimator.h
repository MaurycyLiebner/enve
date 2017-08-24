#ifndef POINTANIMATOR_H
#define POINTANIMATOR_H

#include "movablepoint.h"
#include <QPointF>
#include <QRectF>
#include <QPainter>
#include "Animators/qpointfanimator.h"
#include "transformable.h"
#include "skiaincludes.h"

class BoundingBox;
class NodePoint;
class QSqlQuery;

class AnimatorUpdater;

class PointAnimator :
    public QPointFAnimator,
    public MovablePoint {
public:
    PointAnimator(BoundingBox *parent,
                 const MovablePointType &type,
                 const qreal &radius = 7.5);

    ~PointAnimator() {}

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

    void removeAnimations();

    virtual QPointF getRelativePosAtRelFrame(const int &frame) const;
protected:
};

#endif // POINTANIMATOR_H
