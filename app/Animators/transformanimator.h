#ifndef TRANSFORMANIMATOR_H
#define TRANSFORMANIMATOR_H
#include <QMatrix>
#include "Animators/complexanimator.h"
#include "skia/skiaincludes.h"

class BoundingBox;
class TransformUpdater;
class BoxPathPoint;
class MovablePoint;
class QPointFAnimator;

class BasicTransformAnimator : public ComplexAnimator {
    Q_OBJECT
public:
    BasicTransformAnimator();
    void resetScale();
    void resetTranslation();
    void resetRotation();
    virtual void reset();

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
    virtual QMatrix getCurrentTransformationMatrix();
    virtual QMatrix getRelativeTransformAtRelFrameF(const qreal &relFrame);

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

    bool SWT_isBasicTransformAnimator() const;

    virtual QMatrix getTotalTransformMatrixAtRelFrameF(const qreal &relFrame);
    QMatrix getParentTotalTransformMatrixAtRelFrameF(const qreal &relFrame);

    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice *target);

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

    stdsptr<PropertyUpdater> mTransformUpdater;
public slots:
    virtual void updateTotalTransform(const UpdateReason &reason);
signals:
    void TotalTransformChanged(const UpdateReason &);
};

class Bone;
class BoneTransformAnimator : public BasicTransformAnimator {
public:
    BoneTransformAnimator(Bone *parentBone) {
        mParentBone = parentBone;
    }

    QMatrix getCurrentTransformationMatrix();
    QMatrix getRelativeTransformAtRelFrameF(const qreal &relFrame);
protected:
    Bone *mParentBone = nullptr;
};

class BoxTransformAnimator : public BasicTransformAnimator {
    friend class SelfRef;
protected:
    BoxTransformAnimator(BoundingBox * const parent);
public:
    bool SWT_isBoxTransformAnimator() const { return true; }
    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice *target);
    void updateTotalTransform(const UpdateReason &reason);

    void reset();
    QMatrix getCurrentTransformationMatrix();
    QMatrix getRelativeTransformAtRelFrameF(const qreal &relFrame);

    void resetPivot();

    void setPivotWithoutChangingTransformation(const QPointF &point);
    QPointF getPivot();

    qreal getPivotX();
    qreal getPivotY();

    qreal getOpacity();

    void startOpacityTransform();
    void setOpacity(const qreal &newOpacity);

    MovablePoint *getPivotMovablePoint();
    void startPivotTransform();
    void finishPivotTransform();
    QPointF getPivotAbs();

    qreal getOpacityAtRelFrameF(const qreal &relFrame);

    bool posOrPivotRecording() const;
    bool rotOrScaleOrPivotRecording() const;

    QPointFAnimator *getPivotAnimator() {
        return mPivotAnimator.get();
    }

    QrealAnimator *getOpacityAnimator() {
        return mOpacityAnimator.get();
    }

    BoundingBox *getParentBox() {
        return mParentBox_k;
    }
private:
    bool mPivotAutoAdjust = true;
    BoundingBox * const mParentBox_k;
    stdsptr<BoxPathPoint> mPivotPoint;
    qsptr<QPointFAnimator> mPivotAnimator;
    qsptr<QrealAnimator> mOpacityAnimator;
};

#endif // TRANSFORMANIMATOR_H
