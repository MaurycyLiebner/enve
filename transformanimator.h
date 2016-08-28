#ifndef TRANSFORMANIMATOR_H
#define TRANSFORMANIMATOR_H
#include "complexanimator.h"
#include "qpointfanimator.h"

class TransformAnimator : public ComplexAnimator
{
public:
    TransformAnimator();

    QMatrix getCurrentValue();
    void rotateRelativeToSavedValue(qreal rotRel);
    void translate(qreal dX, qreal dY);
    void scale(qreal sx, qreal sy);
    void scale(qreal sx, qreal sy, QPointF pivot);

    void saveCurrentValue();
    void retrieveSavedValue();

    void finishTransform();

    qreal getYScale();
    qreal getXScale();

    void reset();

    void setPivot(qreal x, qreal y, bool saveUndoRedo);
    void setPivot(QPointF point, bool saveUndoRedo);
    QPointF getPivot();

    qreal dx();
    qreal dy();
    qreal rot();
    qreal xScale();
    qreal yScale();
    QPointF pos();
    void rotateRelativeToSavedValue(qreal rotRel, QPointF pivot);
private:
    QPointF mPivotPoint;
    QPointFAnimator mPosAnimator;
    QPointFAnimator mScaleAnimator;
    QrealAnimator mRotAnimator;
};

#endif // TRANSFORMANIMATOR_H
