#ifndef TRANSFORMANIMATOR_H
#define TRANSFORMANIMATOR_H
#include "Animators/complexanimator.h"
#include "Animators/qpointfanimator.h"
#include "pointanimator.h"
#include "sharedpointerdefs.h"
#include "skqtconversions.h"
class TransformUpdater;
class BoxPathPoint;

class BasicTransformAnimator : public ComplexAnimator {
    Q_OBJECT
public:
    BasicTransformAnimator();
    void resetScale(const bool &finish = false);
    void resetTranslation(const bool &finish = false);
    void resetRotation(const bool &finish = false);
    virtual void reset(const bool &finish = false);

    void setScale(const qreal &sx, const qreal &sy);
    void setPosition(const qreal &x, const qreal &y,
                     const bool &saveUndoRedo);
    void setRotation(const qreal &rot);

    void startRotTransform();
    void startPosTransform();
    void startScaleTransform();

    qreal getYScale();
    qreal getXScale();

    void setRelativePos(const QPointF &relPos,
                        const bool &saveUndoRedo = false);
    void setAbsolutePos(const QPointF &pos,
                        const bool &saveUndoRedo);
    void moveToAbs(const QPointF &absPos);
    void moveByAbs(const QPointF &absTrans);

    void rotateRelativeToSavedValue(const qreal &rotRel);
    void translate(const qreal &dX, const qreal &dY);
    void scale(const qreal &sx, const qreal &sy);
    void moveRelativeToSavedValue(const qreal &dX,
                                  const qreal &dY);
    virtual QMatrix getCurrentTransformationMatrix();
    virtual QMatrix getRelativeTransformAtRelFrame(const int &relFrame);
    virtual QMatrix getRelativeTransformAtRelFrameF(const qreal &relFrame);

    qreal dx();
    qreal dy();
    qreal rot();
    qreal xScale();
    qreal yScale();
    QPointF pos();

    QPointF mapAbsPosToRel(const QPointF &absPos) const {
        return getCombinedTransform().
                inverted().map(absPos);
    }

    QPointF mapRelPosToAbs(const QPointF &relPos) const {
        return getCombinedTransform().map(relPos);
    }

    QPointF mapFromParent(const QPointF &parentRelPos) const {
        return mapAbsPosToRel(
                    mParentTransformAnimator->mapRelPosToAbs(parentRelPos));
    }

    SkPoint mapAbsPosToRel(const SkPoint &absPos) const {
        return QPointFToSkPoint(mapAbsPosToRel(SkPointToQPointF(absPos)));
    }

    SkPoint mapRelPosToAbs(const SkPoint &relPos) const {
        return QPointFToSkPoint(mapRelPosToAbs(SkPointToQPointF(relPos)));
    }

    SkPoint mapFromParent(const SkPoint &parentRelPos) const {
        return QPointFToSkPoint(mapFromParent(SkPointToQPointF(parentRelPos)));
    }

    void scaleRelativeToSavedValue(const qreal &sx,
                                   const qreal &sy,
                                   const QPointF &pivot);
    void rotateRelativeToSavedValue(const qreal &rotRel,
                                    const QPointF &pivot);

    void updateRelativeTransform(const UpdateReason &reason);
    const QMatrix &getCombinedTransform() const;
    const QMatrix &getRelativeTransform() const;

    void setParentTransformAnimator(BasicTransformAnimator *parent);

    bool SWT_isBasicTransformAnimator() { return true; }

    virtual QMatrix getCombinedTransformMatrixAtRelFrame(const int &relFrame);
    QMatrix getParentCombinedTransformMatrixAtRelFrame(const int &relFrame);
    virtual QMatrix getCombinedTransformMatrixAtRelFrameF(const qreal &relFrame);
    QMatrix getParentCombinedTransformMatrixAtRelFrameF(const qreal &relFrame);

    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);

    QPointFAnimator *getPosAnimator() {
        return mPosAnimator.get();
    }

    QPointFAnimator *getScaleAnimator() {
        return mScaleAnimator.get();
    }

    QrealAnimator *getRotAnimator() {
        return mRotAnimator.get();
    }
protected:
    QList<qsptr<BasicTransformAnimator>> mChildBoxes;

    QMatrix mRelTransform;
    QMatrix mCombinedTransform;

    qptr<BasicTransformAnimator> mParentTransformAnimator;

    qsptr<QPointFAnimator> mPosAnimator;
    qsptr<QPointFAnimator> mScaleAnimator;
    qsptr<QrealAnimator> mRotAnimator;

    stdsptr<AnimatorUpdater> mTransformUpdater;
public slots:
    virtual void updateCombinedTransform(const UpdateReason &reason);
signals:
    void combinedTransformChanged(const UpdateReason &);
};

class Bone;
class BoneTransformAnimator : public BasicTransformAnimator {
public:
    BoneTransformAnimator(Bone *parentBone) {
        mParentBone = parentBone;
    }

    QMatrix getCurrentTransformationMatrix();
    QMatrix getRelativeTransformAtRelFrame(const int &relFrame);
    QMatrix getRelativeTransformAtRelFrameF(const qreal &relFrame);
protected:
    Bone *mParentBone = nullptr;
};

class BoxTransformAnimator : public BasicTransformAnimator {
public:
    BoxTransformAnimator(BoundingBox *parent);

    void resetPivot(const bool &finish = false);
    void reset(const bool &finish = false);
    QMatrix getCurrentTransformationMatrix();
    QMatrix getRelativeTransformAtRelFrame(const int &relFrame);
    QMatrix getRelativeTransformAtRelFrameF(const qreal &relFrame);
    QMatrix getCombinedTransformMatrixAtRelFrame(
                                        const int &relFrame);

    void setPivotWithoutChangingTransformation(const QPointF &point,
                                               const bool &saveUndoRedo = false);
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

    bool SWT_isBoxTransformAnimator() { return true; }

    qreal getOpacityAtRelFrame(const int &relFrame);
    qreal getOpacityAtRelFrameF(const qreal &relFrame);

    bool rotOrScaleOrPivotRecording();
    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);

    QPointFAnimator *getPivotAnimator() {
        return mPivotAnimator.get();
    }

    QrealAnimator *getOpacityAnimator() {
        return mOpacityAnimator.get();
    }

    void updateCombinedTransform(const UpdateReason &reason);

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
