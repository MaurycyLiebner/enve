#include "undoredo.h"
#include <QDebug>
#include "MovablePoints/boxpathpoint.h"
#include "PropertyUpdaters/transformupdater.h"
#include "qrealanimator.h"
#include "transformanimator.h"
#include "Boxes/boundingbox.h"
#include "Animators/qpointfanimator.h"
#include "MovablePoints/animatedpoint.h"
#include "skia/skqtconversions.h"

BasicTransformAnimator::BasicTransformAnimator() :
    ComplexAnimator("transformation") {
    mPosAnimator = SPtrCreate(QPointFAnimator)("pos");
    mScaleAnimator = SPtrCreate(QPointFAnimator)("scale");
    mRotAnimator = SPtrCreate(QrealAnimator)("rot");

    mTransformUpdater = SPtrCreate(TransformUpdater)(this);

    mScaleAnimator->setCurrentPointValue(QPointF(1., 1.));
    mScaleAnimator->setPrefferedValueStep(0.05);
    mScaleAnimator->prp_setBlockedUpdater(mTransformUpdater);

    mRotAnimator->qra_setCurrentValue(0.);
    mRotAnimator->prp_setBlockedUpdater(mTransformUpdater);

    mPosAnimator->setCurrentPointValue(QPointF(0., 0.));
    mPosAnimator->prp_setBlockedUpdater(mTransformUpdater);

    ca_addChildAnimator(mPosAnimator);
    ca_addChildAnimator(mRotAnimator);
    ca_addChildAnimator(mScaleAnimator);
}

void BasicTransformAnimator::resetScale() {
    mScaleAnimator->setCurrentPointValue(QPointF(1., 1.));
}

void BasicTransformAnimator::resetTranslation() {
    mPosAnimator->setCurrentPointValue(QPointF(0., 0.));
}

void BasicTransformAnimator::resetRotation() {
    mRotAnimator->qra_setCurrentValue(0.);
}

void BasicTransformAnimator::reset() {
    resetScale();
    resetTranslation();
    resetRotation();
}

void BasicTransformAnimator::setScale(const qreal &sx, const qreal &sy) {
    mScaleAnimator->setCurrentPointValue(QPointF(sx, sy) );
}

void BasicTransformAnimator::setPosition(const qreal &x, const qreal &y) {
    mPosAnimator->setCurrentPointValue(QPointF(x, y));
}

void BasicTransformAnimator::setRotation(const qreal &rot) {
    mRotAnimator->qra_setCurrentValue(rot);
}

void BasicTransformAnimator::startRotTransform() {
    mRotAnimator->prp_startTransform();
}

void BasicTransformAnimator::startPosTransform() {
    mPosAnimator->prp_startTransform();
}

void BasicTransformAnimator::startScaleTransform() {
    mScaleAnimator->prp_startTransform();
}

qreal BasicTransformAnimator::getYScale() {
    return mScaleAnimator->getEffectiveYValue();
}

qreal BasicTransformAnimator::getXScale() {
    return mScaleAnimator->getEffectiveXValue();
}

void BasicTransformAnimator::rotateRelativeToSavedValue(const qreal &rotRel) {
    mRotAnimator->incSavedValueToCurrentValue(rotRel);
}

void BasicTransformAnimator::moveRelativeToSavedValue(const qreal &dX,
                                                      const qreal &dY) {
    mPosAnimator->incSavedValueToCurrentValue(dX, dY);
}

void BasicTransformAnimator::translate(const qreal &dX, const qreal &dY) {
    mPosAnimator->incCurrentValues(dX, dY);
}

void BasicTransformAnimator::scale(const qreal &sx, const qreal &sy) {
    mScaleAnimator->multSavedValueToCurrentValue(sx, sy);
}

qreal BasicTransformAnimator::dx() {
    return mPosAnimator->getEffectiveXValue();
}

qreal BasicTransformAnimator::dy() {
    return mPosAnimator->getEffectiveYValue();
}

qreal BasicTransformAnimator::rot() {
    return mRotAnimator->qra_getCurrentEffectiveValue();
}

qreal BasicTransformAnimator::xScale() {
    return mScaleAnimator->getEffectiveXValue();
}

qreal BasicTransformAnimator::yScale() {
    return mScaleAnimator->getEffectiveYValue();
}

QPointF BasicTransformAnimator::pos() {
    return mPosAnimator->getCurrentEffectivePointValue();
}

QPointF BasicTransformAnimator::mapAbsPosToRel(const QPointF &absPos) const {
    return getCombinedTransform().
            inverted().map(absPos);
}

QPointF BasicTransformAnimator::mapRelPosToAbs(const QPointF &relPos) const {
    return getCombinedTransform().map(relPos);
}

QPointF BasicTransformAnimator::mapFromParent(const QPointF &parentRelPos) const {
    return mapAbsPosToRel(
                mParentTransformAnimator->mapRelPosToAbs(parentRelPos));
}

SkPoint BasicTransformAnimator::mapAbsPosToRel(const SkPoint &absPos) const {
    return qPointToSk(mapAbsPosToRel(skPointToQ(absPos)));
}

SkPoint BasicTransformAnimator::mapRelPosToAbs(const SkPoint &relPos) const {
    return qPointToSk(mapRelPosToAbs(skPointToQ(relPos)));
}

SkPoint BasicTransformAnimator::mapFromParent(const SkPoint &parentRelPos) const {
    return qPointToSk(mapFromParent(skPointToQ(parentRelPos)));
}

QMatrix BasicTransformAnimator::getCurrentTransformationMatrix() {
    QMatrix matrix;

    matrix.translate(mPosAnimator->getEffectiveXValue(),
                     mPosAnimator->getEffectiveYValue());

    matrix.rotate(mRotAnimator->qra_getCurrentEffectiveValue() );
    matrix.scale(mScaleAnimator->getEffectiveXValue(),
                 mScaleAnimator->getEffectiveYValue() );
    return matrix;
}

QMatrix BasicTransformAnimator::getRelativeTransformAtRelFrame(
                                    const int &relFrame) {
    QMatrix matrix;

    matrix.translate(mPosAnimator->getEffectiveXValueAtRelFrame(relFrame),
                     mPosAnimator->getEffectiveYValueAtRelFrame(relFrame));

    matrix.rotate(mRotAnimator->qra_getEffectiveValueAtRelFrame(relFrame) );
    matrix.scale(mScaleAnimator->getEffectiveXValueAtRelFrame(relFrame),
                 mScaleAnimator->getEffectiveYValueAtRelFrame(relFrame) );
    return matrix;
}

QMatrix BasicTransformAnimator::getRelativeTransformAtRelFrameF(
                                    const qreal &relFrame) {
    QMatrix matrix;

    matrix.translate(mPosAnimator->getEffectiveXValueAtRelFrameF(relFrame),
                     mPosAnimator->getEffectiveYValueAtRelFrameF(relFrame));

    matrix.rotate(mRotAnimator->qra_getEffectiveValueAtRelFrameF(relFrame) );
    matrix.scale(mScaleAnimator->getEffectiveXValueAtRelFrameF(relFrame),
                 mScaleAnimator->getEffectiveYValueAtRelFrameF(relFrame) );
    return matrix;
}

void BasicTransformAnimator::moveByAbs(const QPointF &absTrans) {
    moveToAbs(mParentTransformAnimator->mapRelPosToAbs(mPosAnimator->getSavedPointValue()) +
              absTrans);
}

void BasicTransformAnimator::moveToAbs(const QPointF &absPos) {
    setAbsolutePos(absPos);
}

void BasicTransformAnimator::setAbsolutePos(const QPointF &pos) {
    setRelativePos(mParentTransformAnimator->mapAbsPosToRel(pos));
}

void BasicTransformAnimator::setRelativePos(const QPointF &relPos) {
    mPosAnimator->setCurrentPointValue(relPos);
}

void BasicTransformAnimator::rotateRelativeToSavedValue(const qreal &rotRel,
                                                        const QPointF &pivot) {
    QMatrix matrix;
    matrix.translate(pivot.x(),
                     pivot.y());
    matrix.rotate(rotRel);
    matrix.translate(-pivot.x() + mPosAnimator->getSavedXValue(),
                     -pivot.y() + mPosAnimator->getSavedYValue() );
    rotateRelativeToSavedValue(rotRel);
    mPosAnimator->setCurrentPointValue(QPointF(matrix.dx(), matrix.dy()) );
}

void BasicTransformAnimator::updateRelativeTransform(const UpdateReason &reason) {
    mRelTransform = getCurrentTransformationMatrix();
    updateCombinedTransform(reason);
}

void BasicTransformAnimator::updateCombinedTransform(const UpdateReason &reason) {
    if(mParentTransformAnimator.isNull()) {
        mCombinedTransform = mRelTransform;
    } else {
        mCombinedTransform = mRelTransform *
                             mParentTransformAnimator->getCombinedTransform();
    }
    emit combinedTransformChanged(reason);
}

const QMatrix &BasicTransformAnimator::getCombinedTransform() const {
    return mCombinedTransform;
}

const QMatrix &BasicTransformAnimator::getRelativeTransform() const {
    return mRelTransform;
}

void BasicTransformAnimator::setParentTransformAnimator(
        BasicTransformAnimator* parent) {
    if(mParentTransformAnimator) {
        disconnect(mParentTransformAnimator,
                   &BasicTransformAnimator::combinedTransformChanged,
                   this,
                   &BasicTransformAnimator::updateCombinedTransform);
    }
    mParentTransformAnimator = parent;
    if(parent) {
        connect(parent,
                &BasicTransformAnimator::combinedTransformChanged,
                this,
                &BasicTransformAnimator::updateCombinedTransform);
    }
    updateCombinedTransform(Animator::USER_CHANGE);
}

bool BasicTransformAnimator::SWT_isBasicTransformAnimator() const { return true; }

void BasicTransformAnimator::scaleRelativeToSavedValue(const qreal &sx,
                                                       const qreal &sy,
                                                      const QPointF &pivot) {
    QMatrix matrix;

    matrix.translate(pivot.x(), pivot.y());
    matrix.rotate(mRotAnimator->qra_getCurrentEffectiveValue());
    matrix.scale(sx, sy);
    matrix.rotate(-mRotAnimator->qra_getCurrentEffectiveValue());
    matrix.translate(-pivot.x() + mPosAnimator->getSavedXValue(),
                     -pivot.y() + mPosAnimator->getSavedYValue());

    scale(sx, sy);
    mPosAnimator->setCurrentPointValue(QPointF(matrix.dx(), matrix.dy()) );
}

BoxTransformAnimator::BoxTransformAnimator(BoundingBox *parent) :
    BasicTransformAnimator(), mParentBox_k(parent) {
    mOpacityAnimator = SPtrCreate(QrealAnimator)("opacity");
    mPivotAnimator = SPtrCreate(QPointFAnimator)("pivot");
    mPivotAnimator->setCurrentPointValue(QPointF(0., 0.));
    mPivotAnimator->prp_setBlockedUpdater(mTransformUpdater);
    mOpacityAnimator->qra_setValueRange(0., 100.);
    mOpacityAnimator->setPrefferedValueStep(5.);
    mOpacityAnimator->qra_setCurrentValue(100.);
    mOpacityAnimator->graphFixMinMaxValues();
    mOpacityAnimator->prp_setBlockedUpdater(mTransformUpdater);

    ca_addChildAnimator(mPivotAnimator);
    ca_addChildAnimator(mOpacityAnimator);

    mPivotPoint = SPtrCreate(BoxPathPoint)(mPivotAnimator.get(),
                                           this);
}

MovablePoint *BoxTransformAnimator::getPivotMovablePoint() {
    return mPivotPoint.get();
}

void BoxTransformAnimator::resetPivot() {
    mPivotAnimator->setCurrentPointValue(QPointF(0., 0.));
}

void BoxTransformAnimator::reset() {
    BasicTransformAnimator::reset();
    resetPivot();
}

void BoxTransformAnimator::startOpacityTransform() {
    mOpacityAnimator->prp_startTransform();
}

void BoxTransformAnimator::setOpacity(const qreal &newOpacity) {
    mOpacityAnimator->qra_setCurrentValue(newOpacity);
}

void BoxTransformAnimator::startPivotTransform() {
    if(!mPosAnimator->prp_isDescendantRecording()) {
        mPosAnimator->prp_startTransform();
    }
    mPivotAnimator->prp_startTransform();
}

void BoxTransformAnimator::finishPivotTransform() {
    if(!mPosAnimator->prp_isDescendantRecording()) {
        mPosAnimator->prp_finishTransform();
    }
    mPivotAnimator->prp_finishTransform();
}

void BoxTransformAnimator::setPivotWithoutChangingTransformation(const QPointF &point) {
    QMatrix currentMatrix;
    qreal pivotX = mPivotAnimator->getEffectiveXValue();
    qreal pivotY = mPivotAnimator->getEffectiveYValue();
    currentMatrix.translate(pivotX + mPosAnimator->getEffectiveXValue(),
                            pivotY + mPosAnimator->getEffectiveYValue());

    currentMatrix.rotate(mRotAnimator->qra_getCurrentEffectiveValue() );
    currentMatrix.scale(mScaleAnimator->getEffectiveXValue(),
                        mScaleAnimator->getEffectiveYValue() );

    currentMatrix.translate(-pivotX, -pivotY);

    QMatrix futureMatrix;
    futureMatrix.translate(point.x() + mPosAnimator->getEffectiveXValue(),
                           point.y() + mPosAnimator->getEffectiveYValue());

    futureMatrix.rotate(mRotAnimator->qra_getCurrentEffectiveValue() );
    futureMatrix.scale(mScaleAnimator->getEffectiveXValue(),
                       mScaleAnimator->getEffectiveYValue() );

    futureMatrix.translate(-point.x(), -point.y());


    mPosAnimator->incAllValues(currentMatrix.dx() - futureMatrix.dx(),
                               currentMatrix.dy() - futureMatrix.dy());

    mPivotAnimator->setCurrentPointValue(point);
}

QPointF BoxTransformAnimator::getPivot() {
    return mPivotAnimator->getCurrentEffectivePointValue();
}

QPointF BoxTransformAnimator::getPivotAbs() {
    return mPivotPoint->getAbsolutePos();
}

qreal BoxTransformAnimator::getOpacityAtRelFrame(const int &relFrame) {
    return mOpacityAnimator->qra_getEffectiveValueAtRelFrame(relFrame);
}

qreal BoxTransformAnimator::getOpacityAtRelFrameF(const qreal &relFrame) {
    return mOpacityAnimator->qra_getEffectiveValueAtRelFrameF(relFrame);
}

bool BoxTransformAnimator::rotOrScaleOrPivotRecording() {
    return mRotAnimator->prp_isDescendantRecording() ||
           mScaleAnimator->prp_isDescendantRecording() ||
           mPivotAnimator->prp_isDescendantRecording();
}

void BoxTransformAnimator::updateCombinedTransform(const UpdateReason &reason) {
    BasicTransformAnimator::updateCombinedTransform(reason);

    mParentBox_k->updateDrawRenderContainerTransform();
    mParentBox_k->scheduleUpdate(reason);
    mParentBox_k->requestGlobalPivotUpdateIfSelected();
}

qreal BoxTransformAnimator::getPivotX() {
    return mPivotAnimator->getEffectiveXValue();
}

qreal BoxTransformAnimator::getPivotY() {
    return mPivotAnimator->getEffectiveYValue();
}

qreal BoxTransformAnimator::getOpacity() {
    return mOpacityAnimator->qra_getCurrentValue();
}

QMatrix BoxTransformAnimator::getCurrentTransformationMatrix() {
    QMatrix matrix;
    qreal pivotX = mPivotAnimator->getEffectiveXValue();
    qreal pivotY = mPivotAnimator->getEffectiveYValue();

    matrix.translate(pivotX + mPosAnimator->getEffectiveXValue(),
                     pivotY + mPosAnimator->getEffectiveYValue());

    matrix.rotate(mRotAnimator->qra_getCurrentEffectiveValue() );
    matrix.scale(mScaleAnimator->getEffectiveXValue(),
                 mScaleAnimator->getEffectiveYValue() );

    matrix.translate(-pivotX, -pivotY);
    return matrix;
}

QMatrix BoxTransformAnimator::getRelativeTransformAtRelFrame(
                                    const int &relFrame) {
    QMatrix matrix;
    qreal pivotX = mPivotAnimator->getEffectiveXValueAtRelFrame(relFrame);
    qreal pivotY = mPivotAnimator->getEffectiveYValueAtRelFrame(relFrame);

    matrix.translate(pivotX + mPosAnimator->getEffectiveXValueAtRelFrame(relFrame),
                     pivotY + mPosAnimator->getEffectiveYValueAtRelFrame(relFrame));

    matrix.rotate(mRotAnimator->qra_getEffectiveValueAtRelFrame(relFrame) );
    matrix.scale(mScaleAnimator->getEffectiveXValueAtRelFrame(relFrame),
                 mScaleAnimator->getEffectiveYValueAtRelFrame(relFrame) );

    matrix.translate(-pivotX, -pivotY);
    return matrix;
}

QMatrix BoxTransformAnimator::getRelativeTransformAtRelFrameF(
                                    const qreal &relFrame) {
    QMatrix matrix;
    qreal pivotX = mPivotAnimator->getEffectiveXValueAtRelFrameF(relFrame);
    qreal pivotY = mPivotAnimator->getEffectiveYValueAtRelFrameF(relFrame);

    matrix.translate(pivotX + mPosAnimator->getEffectiveXValueAtRelFrameF(relFrame),
                     pivotY + mPosAnimator->getEffectiveYValueAtRelFrameF(relFrame));

    matrix.rotate(mRotAnimator->qra_getEffectiveValueAtRelFrameF(relFrame) );
    matrix.scale(mScaleAnimator->getEffectiveXValueAtRelFrameF(relFrame),
                 mScaleAnimator->getEffectiveYValueAtRelFrameF(relFrame) );

    matrix.translate(-pivotX, -pivotY);
    return matrix;
}

QMatrix BoxTransformAnimator::getCombinedTransformMatrixAtRelFrame(
        const int &relFrame) {
    if(mParentTransformAnimator.data() == nullptr) {
        return mParentBox_k->getRelativeTransformAtRelFrame(relFrame);
    } else {
        int absFrame = prp_relFrameToAbsFrame(relFrame);
        int parentRelFrame =
                mParentTransformAnimator->prp_absFrameToRelFrame(absFrame);
        return mParentBox_k->getRelativeTransformAtRelFrame(relFrame)*
                mParentTransformAnimator->
                getCombinedTransformMatrixAtRelFrame(parentRelFrame);
    }
}

QMatrix BasicTransformAnimator::getParentCombinedTransformMatrixAtRelFrame(
        const int &relFrame) {
    if(mParentTransformAnimator.data() == nullptr) {
        return QMatrix();
    } else {
        int absFrame = prp_relFrameToAbsFrame(relFrame);
        int parentRelFrame =
                mParentTransformAnimator->prp_absFrameToRelFrame(absFrame);
        return mParentTransformAnimator->
                getCombinedTransformMatrixAtRelFrame(parentRelFrame);
    }
}

QMatrix BasicTransformAnimator::getCombinedTransformMatrixAtRelFrame(
        const int &relFrame) {
    if(mParentTransformAnimator.data() == nullptr) {
        return getRelativeTransformAtRelFrame(relFrame);
    } else {
        int absFrame = prp_relFrameToAbsFrame(relFrame);
        int parentRelFrame =
                mParentTransformAnimator->prp_absFrameToRelFrame(absFrame);
        return getRelativeTransformAtRelFrame(relFrame)*
                mParentTransformAnimator->
                    getCombinedTransformMatrixAtRelFrame(parentRelFrame);
    }
}

QMatrix BasicTransformAnimator::getParentCombinedTransformMatrixAtRelFrameF(
        const qreal &relFrame) {
    if(mParentTransformAnimator.data() == nullptr) {
        return QMatrix();
    } else {
        qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
        qreal parentRelFrame =
                mParentTransformAnimator->prp_absFrameToRelFrameF(absFrame);
        return mParentTransformAnimator->
                getCombinedTransformMatrixAtRelFrameF(parentRelFrame);
    }
}

QPointFAnimator *BasicTransformAnimator::getPosAnimator() {
    return mPosAnimator.get();
}

QPointFAnimator *BasicTransformAnimator::getScaleAnimator() {
    return mScaleAnimator.get();
}

QrealAnimator *BasicTransformAnimator::getRotAnimator() {
    return mRotAnimator.get();
}

QMatrix BasicTransformAnimator::getCombinedTransformMatrixAtRelFrameF(
        const qreal &relFrame) {
    if(mParentTransformAnimator.data() == nullptr) {
        return getRelativeTransformAtRelFrameF(relFrame);
    } else {
        qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
        qreal parentRelFrame =
                mParentTransformAnimator->prp_absFrameToRelFrameF(absFrame);
        return getRelativeTransformAtRelFrameF(relFrame)*
                mParentTransformAnimator->
                    getCombinedTransformMatrixAtRelFrameF(parentRelFrame);
    }
}

#include "Boxes/bone.h"
QMatrix BoneTransformAnimator::BoneTransformAnimator::getCurrentTransformationMatrix() {
    QMatrix matrix;

    QPointF rootRelPos = mParentBone->getRootRelPos();
    matrix.translate(rootRelPos.x() + mPosAnimator->getEffectiveXValue(),
                     rootRelPos.y() + mPosAnimator->getEffectiveYValue());

    matrix.rotate(mRotAnimator->qra_getCurrentEffectiveValue() );
    matrix.scale(mScaleAnimator->getEffectiveXValue(),
                 mScaleAnimator->getEffectiveYValue() );
    matrix.translate(-rootRelPos.x(), -rootRelPos.y());
    return matrix;
}

QMatrix BoneTransformAnimator::getRelativeTransformAtRelFrame(
                                    const int &relFrame) {
    QMatrix matrix;
    QPointF rootRelPos = mParentBone->getRootRelPos();
    matrix.translate(rootRelPos.x() + mPosAnimator->getEffectiveXValueAtRelFrame(relFrame),
                     rootRelPos.y() + mPosAnimator->getEffectiveYValueAtRelFrame(relFrame));

    matrix.rotate(mRotAnimator->qra_getEffectiveValueAtRelFrame(relFrame) );
    matrix.scale(mScaleAnimator->getEffectiveXValueAtRelFrame(relFrame),
                 mScaleAnimator->getEffectiveYValueAtRelFrame(relFrame) );
    matrix.translate(-rootRelPos.x(), -rootRelPos.y());

    return matrix;
}

QMatrix BoneTransformAnimator::getRelativeTransformAtRelFrameF(
                                    const qreal &relFrame) {
    QMatrix matrix;
    QPointF rootRelPos = mParentBone->getRootRelPos();
    matrix.translate(rootRelPos.x() + mPosAnimator->getEffectiveXValueAtRelFrameF(relFrame),
                     rootRelPos.y() + mPosAnimator->getEffectiveYValueAtRelFrameF(relFrame));

    matrix.rotate(mRotAnimator->qra_getEffectiveValueAtRelFrameF(relFrame) );
    matrix.scale(mScaleAnimator->getEffectiveXValueAtRelFrameF(relFrame),
                 mScaleAnimator->getEffectiveYValueAtRelFrameF(relFrame) );
    matrix.translate(-rootRelPos.x(), -rootRelPos.y());

    return matrix;
}
