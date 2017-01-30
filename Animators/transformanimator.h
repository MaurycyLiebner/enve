#ifndef TRANSFORMANIMATOR_H
#define TRANSFORMANIMATOR_H
#include "Animators/complexanimator.h"
#include "Animators/qpointfanimator.h"

class TransformAnimator : public ComplexAnimator
{
public:
    TransformAnimator();

    void resetScale(bool finish = false);
    void resetTranslation(bool finish = false);
    void resetRotation(bool finish = false);
    void reset(bool finish = false);

    QMatrix getCurrentValue();
    void rotateRelativeToSavedValue(qreal rotRel);
    void translate(qreal dX, qreal dY);
    void scale(qreal sx, qreal sy);
    void scaleRelativeToSavedValue(qreal sx, qreal sy, QPointF pivot);

    void setScale(qreal sx, qreal sy);
    void setPosition(qreal x, qreal y);
    void setRotation(qreal rot);

    qreal getYScale();
    qreal getXScale();

    void setPivotWithoutChangingTransformation(qreal x, qreal y);
    void setPivotWithoutChangingTransformation(QPointF point, bool finish = false);
    QPointF getPivot();

    qreal dx();
    qreal dy();
    qreal rot();
    qreal xScale();
    qreal yScale();
    QPointF pos();

    qreal getPivotX();
    qreal getPivotY();

    qreal getOpacity();

    void rotateRelativeToSavedValue(qreal rotRel, QPointF pivot);
    void startRotTransform();
    void startPosTransform();
    void startScaleTransform();
    void startOpacityTransform();
    void setOpacity(qreal newOpacity);
    void moveRelativeToSavedValue(qreal dX, qreal dY);
    void copyTransformationTo(TransformAnimator *targetAnimator);
    void setPivot(QPointF point, bool finish = false);
    int saveToSql(QSqlQuery *query);
    void loadFromSql(int transformAnimatorId);
    void setBaseTransformation(const QMatrix &matrix);
    bool hasBaseTransformation();

    void makeDuplicate(QrealAnimator *target);

    void duplicatePivotAnimatorFrom(QPointFAnimator *source);
    void duplicatePosAnimatorFrom(QPointFAnimator *source);
    void duplicateScaleAnimatorFrom(QPointFAnimator *source);
    void duplicateRotAnimatorFrom(QrealAnimator *source);
    void duplicateOpacityAnimatorFrom(QrealAnimator *source);
private:
    bool mBaseTransformationSet = false;
    QMatrix mBaseTransformation;
    QPointFAnimator mPivotAnimator;
    QPointFAnimator mPosAnimator;
    QPointFAnimator mScaleAnimator;
    QrealAnimator mRotAnimator;
    QrealAnimator mOpacityAnimator;
};

#endif // TRANSFORMANIMATOR_H
