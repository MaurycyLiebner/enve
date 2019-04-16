#ifndef TRANSFORMANIMATOR_H
#define TRANSFORMANIMATOR_H
#include <QMatrix>
#include "Animators/complexanimator.h"
#include "skia/skiaincludes.h"

class TransformUpdater;
class BoxPathPoint;
class MovablePoint;
class QPointFAnimator;

class BasicTransformAnimator : public ComplexAnimator {
    Q_OBJECT
    friend class SeflRef;
protected:
    BasicTransformAnimator();
public:
    virtual void reset();
    virtual QMatrix getCurrentTransform();
    virtual QMatrix getRelativeTransform(const qreal &relFrame);
    virtual QMatrix getTotalTransformAtRelFrameF(const qreal &relFrame);

    bool SWT_isBasicTransformAnimator() const;

    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice *target);

    void resetScale();
    void resetTranslation();
    void resetRotation();

    void setScale(const qreal &sx, const qreal &sy);
    void setPosition(const qreal &x, const qreal &y);
    void setRotation(const qreal &rot);

    void startRotTransform();
    void startPosTransform();
    void startScaleTransform();

    qreal getYScale();
    qreal getXScale();

    void setRelativePos(const QPointF &relPos);
    void setAbsolutePos(const QPointF &pos);
    void moveToAbs(const QPointF &absPos);
    void moveByAbs(const QPointF &absTrans);

    void rotateRelativeToSavedValue(const qreal &rotRel);
    void translate(const qreal &dX, const qreal &dY);
    void scale(const qreal &sx, const qreal &sy);
    void moveRelativeToSavedValue(const qreal &dX,
                                  const qreal &dY);

    qreal dx();
    qreal dy();
    qreal rot();
    qreal xScale();
    qreal yScale();
    QPointF pos();

    QPointF mapAbsPosToRel(const QPointF &absPos) const;
    QPointF mapRelPosToAbs(const QPointF &relPos) const;

    QPointF mapFromParent(const QPointF &parentRelPos) const;
    SkPoint mapAbsPosToRel(const SkPoint &absPos) const;
    SkPoint mapRelPosToAbs(const SkPoint &relPos) const;
    SkPoint mapFromParent(const SkPoint &parentRelPos) const;

    void scaleRelativeToSavedValue(const qreal &sx,
                                   const qreal &sy,
                                   const QPointF &pivot);
    void rotateRelativeToSavedValue(const qreal &rotRel,
                                    const QPointF &pivot);

    void updateRelativeTransform(const UpdateReason &reason);
    const QMatrix &getTotalTransform() const;
    const QMatrix &getRelativeTransform() const;

    void setParentTransformAnimator(BasicTransformAnimator *parent);

    QMatrix getParentTotalTransformAtRelFrame(const qreal &relFrame);

    QPointFAnimator *getPosAnimator();
    QPointFAnimator *getScaleAnimator();
    QrealAnimator *getRotAnimator();
protected:
    QList<qsptr<BasicTransformAnimator>> mChildBoxes;

    QMatrix mRelTransform;
    QMatrix mTotalTransform;

    qptr<BasicTransformAnimator> mParentTransformAnimator;

    qsptr<QPointFAnimator> mPosAnimator;
    qsptr<QPointFAnimator> mScaleAnimator;
    qsptr<QrealAnimator> mRotAnimator;
public slots:
    void updateTotalTransform(const UpdateReason &reason);
signals:
    void totalTransformChanged(const UpdateReason &);
};

class BoxTransformAnimator : public BasicTransformAnimator {
    friend class SelfRef;
protected:
    BoxTransformAnimator();
public:
    void reset();
    QMatrix getCurrentTransform();
    QMatrix getRelativeTransform(const qreal &relFrame);

    bool SWT_isBoxTransformAnimator() const { return true; }
    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice *target);

    void resetPivot();
    void setPivotFixedTransform(const QPointF &point);

    QPointF getPivot();
    qreal getPivotX();
    qreal getPivotY();

    void setShear(const qreal& shearX, const qreal& shearY);

    qreal getOpacity();

    void startOpacityTransform();
    void setOpacity(const qreal &newOpacity);

    MovablePoint *getPivotMovablePoint();
    void startPivotTransform();
    void finishPivotTransform();
    QPointF getPivotAbs();

    qreal getOpacity(const qreal &relFrame);

    bool posOrPivotRecording() const;
    bool rotOrScaleOrPivotRecording() const;

    QPointFAnimator *getShearAnimator() {
        return mShearAnimator.get();
    }

    QPointFAnimator *getPivotAnimator() {
        return mPivotAnimator.get();
    }

    QrealAnimator *getOpacityAnimator() {
        return mOpacityAnimator.get();
    }

    void setPivotAutoAdjust(const bool& autoAdjust) {
        mPivotAutoAdjust = autoAdjust;
    }

    bool getPivotAutoadjust() const {
        return mPivotAutoAdjust;
    }
private:
    bool mPivotAutoAdjust = true;

    stdsptr<BoxPathPoint> mPivotPoint;
    qsptr<QPointFAnimator> mPivotAnimator;
    qsptr<QPointFAnimator> mShearAnimator;
    qsptr<QrealAnimator> mOpacityAnimator;
};

#endif // TRANSFORMANIMATOR_H
