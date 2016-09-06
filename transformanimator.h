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

    void setScale(qreal sx, qreal sy);
    void setPosition(qreal x, qreal y);
    void setRotation(qreal rot);

    void startTransform();
    void retrieveSavedValue();

    void finishTransform(bool record);

    void cancelTransform();

    qreal getYScale();
    qreal getXScale();

    void reset();

    void setPivot(qreal x, qreal y);
    void setPivot(QPointF point);
    QPointF getPivot();

    qreal dx();
    qreal dy();
    qreal rot();
    qreal xScale();
    qreal yScale();
    QPointF pos();

    qreal getPivotX();
    qreal getPivotY();

    void rotateRelativeToSavedValue(qreal rotRel, QPointF pivot);
    void startRotTransform();
    void startPosTransform();
    void startScaleTransform();

    void setConnectedToMainWindow(ConnectedToMainWindow *connected);

    void setUpdater(AnimatorUpdater *updater);

    void setFrame(int frame);

    void sortKeys();
    void updateKeysPath();
private:
    QPointF mPivotPoint;
    QPointFAnimator mPosAnimator;
    QPointFAnimator mScaleAnimator;
    QrealAnimator mRotAnimator;
};

#endif // TRANSFORMANIMATOR_H
