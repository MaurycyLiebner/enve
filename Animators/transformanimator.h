#ifndef TRANSFORMANIMATOR_H
#define TRANSFORMANIMATOR_H
#include "Animators/complexanimator.h"
#include "Animators/qpointfanimator.h"
#include "movablepoint.h"

class BasicTransformAnimator : public ComplexAnimator
{
public:
    void resetScale(const bool &finish = false);
    void resetTranslation(const bool &finish = false);
    void resetRotation(const bool &finish = false);
    virtual void reset(const bool &finish = false);

    void setScale(const qreal &sx, const qreal &sy);
    void setPosition(const qreal &x, const qreal &y);
    void setRotation(const qreal &rot);

    void startRotTransform();
    void startPosTransform();
    void startScaleTransform();

    qreal getYScale();
    qreal getXScale();

    void setRelativePos(const QPointF &relPos,
                        const bool &saveUndoRedo = false);
    void setAbsolutePos(const QMatrix &combinedTrans,
                        const QPointF &pos,
                        const bool &saveUndoRedo);
    void moveToAbs(const QMatrix &combinedTrans,
                   const QPointF &absPos);
    void moveByAbs(const QMatrix &combinedTrans,
                   const QPointF &absTrans);

    void rotateRelativeToSavedValue(const qreal &rotRel);
    void translate(const qreal &dX, const qreal &dY);
    void scale(const qreal &sx, const qreal &sy);
    void moveRelativeToSavedValue(const qreal &dX,
                                  const qreal &dY);
    virtual QMatrix getCurrentTransformationMatrix();

    qreal dx();
    qreal dy();
    qreal rot();
    qreal xScale();
    qreal yScale();
    QPointF pos();

    void duplicatePosAnimatorFrom(QPointFAnimator *source);
    void duplicateScaleAnimatorFrom(QPointFAnimator *source);
    void duplicateRotAnimatorFrom(QrealAnimator *source);

    void scaleRelativeToSavedValue(const qreal &sx,
                                   const qreal &sy,
                                   const QPointF &pivot);
    void rotateRelativeToSavedValue(const qreal &rotRel,
                                    const QPointF &pivot);
protected:
    QSharedPointer<QPointFAnimator> mPosAnimator =
            (new QPointFAnimator)->ref<QPointFAnimator>();
    QSharedPointer<QPointFAnimator> mScaleAnimator =
            (new QPointFAnimator)->ref<QPointFAnimator>();
    QSharedPointer<QrealAnimator> mRotAnimator =
            (new QrealAnimator)->ref<QrealAnimator>();
};

class TransformAnimator : public BasicTransformAnimator
{
public:
    TransformAnimator(BoundingBox *parent);

    void resetPivot(const bool &finish = false);
    void reset(const bool &finish = false);

    QMatrix getCurrentTransformationMatrix();

    void setPivotWithoutChangingTransformation(QPointF point,
                                               const bool &finish = false);
    QPointF getPivot();

    qreal getPivotX();
    qreal getPivotY();

    qreal getOpacity();

    void startOpacityTransform();
    void setOpacity(const qreal &newOpacity);

    void setPivot(const QPointF &point,
                  const bool &finish = false);
    int prp_saveToSql(QSqlQuery *query,
                      const int &parentId = 0);
    void prp_loadFromSql(const int &transformAnimatorId);

    void makeDuplicate(TransformAnimator *target);
    Property *makeDuplicate() {
        return NULL;
    }

    void duplicatePivotAnimatorFrom(QPointFAnimator *source);
    void duplicateOpacityAnimatorFrom(QrealAnimator *source);

    MovablePoint *getPivotMovablePoint();
    void pivotTransformStarted();
    void pivotTransformFinished();
private:
    QSharedPointer<MovablePoint> mPivotAnimator;
    QSharedPointer<QrealAnimator> mOpacityAnimator =
            (new QrealAnimator)->ref<QrealAnimator>();
};

#endif // TRANSFORMANIMATOR_H
