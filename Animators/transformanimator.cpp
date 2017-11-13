#include "undoredo.h"
#include <QDebug>
#include "boxpathpoint.h"
#include "animatorupdater.h"
#include "qrealanimator.h"
#include "transformanimator.h"
#include "Boxes/boundingbox.h"

BasicTransformAnimator::BasicTransformAnimator() :
    ComplexAnimator() {
    mRotAnimator = (new QrealAnimator)->ref<QrealAnimator>();
    mTransformUpdater = (new TransformUpdater(this))->ref<TransformUpdater>();

    prp_setName("transformation");
    mScaleAnimator->prp_setName("scale");
    mScaleAnimator->setCurrentPointValue(QPointF(1., 1.));
    mScaleAnimator->setPrefferedValueStep(0.05);
    mScaleAnimator->prp_setBlockedUpdater(mTransformUpdater.get());

    mRotAnimator->prp_setName("rot");
    mRotAnimator->qra_setCurrentValue(0.);
    mRotAnimator->prp_setBlockedUpdater(mTransformUpdater.get());

    mPosAnimator->prp_setName("trans");
    mPosAnimator->setCurrentPointValue(QPointF(0., 0.) );
    mPosAnimator->prp_setBlockedUpdater(mTransformUpdater.get());

    ca_addChildAnimator(mPosAnimator.data());
    ca_addChildAnimator(mRotAnimator.data());
    ca_addChildAnimator(mScaleAnimator.data());
}

void BasicTransformAnimator::resetScale(const bool &finish) {
    mScaleAnimator->setCurrentPointValue(QPointF(1., 1.), finish);
}

void BasicTransformAnimator::resetTranslation(const bool &finish) {
    mPosAnimator->setCurrentPointValue(QPointF(0., 0.), finish);
}

void BasicTransformAnimator::resetRotation(const bool &finish) {
    mRotAnimator->qra_setCurrentValue(0., finish);
}

void BasicTransformAnimator::reset(const bool &finish) {
    resetScale(finish);
    resetTranslation(finish);
    resetRotation(finish);
}

void BasicTransformAnimator::setScale(const qreal &sx, const qreal &sy) {
    mScaleAnimator->setCurrentPointValue(QPointF(sx, sy) );
}

void BasicTransformAnimator::setPosition(const qreal &x, const qreal &y,
                                         const bool &saveUndoRedo) {
    mPosAnimator->setCurrentPointValue(QPointF(x, y), saveUndoRedo);
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

void BasicTransformAnimator::duplicatePosAnimatorFrom(
                        QPointFAnimator *source) {
    source->makeDuplicate(mPosAnimator.data());
}

void BasicTransformAnimator::duplicateScaleAnimatorFrom(
                        QPointFAnimator *source) {
    source->makeDuplicate(mScaleAnimator.data());
}

void BasicTransformAnimator::duplicateRotAnimatorFrom(
                        QrealAnimator *source) {
    source->makeDuplicate(mRotAnimator.data());
}

void BasicTransformAnimator::moveByAbs(
                        const QMatrix &combinedTrans,
                        const QPointF &absTrans) {
    moveToAbs(combinedTrans,
              combinedTrans.map(mPosAnimator->getSavedPointValue()) +
              absTrans);
}

void BasicTransformAnimator::moveToAbs(
                        const QMatrix &combinedTrans,
                        const QPointF &absPos) {
    setAbsolutePos(combinedTrans, absPos, false);
}

void BasicTransformAnimator::setAbsolutePos(
                        const QMatrix &combinedTrans,
                        const QPointF &pos,
                        const bool &saveUndoRedo) {
    QPointF newPos = combinedTrans.inverted().map(pos);
    setRelativePos(newPos, saveUndoRedo);
}

void BasicTransformAnimator::setRelativePos(const QPointF &relPos,
                                            const bool &saveUndoRedo) {
    mPosAnimator->setCurrentPointValue(relPos, saveUndoRedo);
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

void BasicTransformAnimator::updateRelativeTransform() {
    mRelTransform = getCurrentTransformationMatrix();
    updateCombinedTransform();
}

void BasicTransformAnimator::updateCombinedTransform() {
    if(mParentTransformAnimator.isNull()) {
        mCombinedTransform = mRelTransform;
    } else {
        mCombinedTransform = mRelTransform *
                             mParentTransformAnimator->getCombinedTransform();
    }
    emit combinedTransformChanged();
}

const QMatrix &BasicTransformAnimator::getCombinedTransform() const {
    return mCombinedTransform;
}

const QMatrix &BasicTransformAnimator::getRelativeTransform() const {
    return mRelTransform;
}

void BasicTransformAnimator::setParentTransformAnimator(
                                BasicTransformAnimator *parent) {
    if(!mParentTransformAnimator.isNull()) {
        disconnect(parent, SIGNAL(combinedTransformChanged()),
                   this, SLOT(updateCombinedTransform()));
    }
    if(parent == NULL) {
        mParentTransformAnimator.reset();
    } else {
        mParentTransformAnimator = parent->ref<BasicTransformAnimator>();
        connect(parent, SIGNAL(combinedTransformChanged()),
                this, SLOT(updateCombinedTransform()));
    }
    updateCombinedTransform();
}

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

void BasicTransformAnimator::makeDuplicate(BasicTransformAnimator *target) {
    target->duplicatePosAnimatorFrom(mPosAnimator.data());
    target->duplicateScaleAnimatorFrom(mScaleAnimator.data());
    target->duplicateRotAnimatorFrom(mRotAnimator.data());
}

BoxTransformAnimator::BoxTransformAnimator(BoundingBox *parent) :
    BasicTransformAnimator() {
    mParentBox = parent;
    mOpacityAnimator = (new QrealAnimator)->ref<QrealAnimator>();
    mPivotAnimator = (new BoxPathPoint(parent))->ref<PointAnimator>();
    mPivotAnimator->prp_setName("pivot");
    mPivotAnimator->setCurrentPointValue(QPointF(0., 0.) );
    mPivotAnimator->prp_setBlockedUpdater(mTransformUpdater.get());
    mOpacityAnimator->prp_setName("opacity");
    mOpacityAnimator->qra_setValueRange(0., 100.);
    mOpacityAnimator->setPrefferedValueStep(5.);
    mOpacityAnimator->qra_setCurrentValue(100.);
    mOpacityAnimator->freezeMinMaxValues();
    mOpacityAnimator->prp_setBlockedUpdater(mTransformUpdater.get());

    ca_addChildAnimator(mPivotAnimator.data());
    ca_addChildAnimator(mOpacityAnimator.data());

    prp_setBlockedUpdater(new TransUpdater(parent) );
}

MovablePoint *BoxTransformAnimator::getPivotMovablePoint() {
    return mPivotAnimator.data();
}

void BoxTransformAnimator::resetPivot(const bool &finish) {
    mPivotAnimator->setCurrentPointValue(QPointF(0., 0.), finish);
}

void BoxTransformAnimator::reset(const bool &finish) {
    BasicTransformAnimator::reset(finish);
    resetPivot(finish);
}

void BoxTransformAnimator::startOpacityTransform() {
    mOpacityAnimator->prp_startTransform();
}

void BoxTransformAnimator::setOpacity(const qreal &newOpacity) {
    mOpacityAnimator->qra_setCurrentValue(newOpacity);
}

void BoxTransformAnimator::pivotTransformStarted() {
    if(!mPosAnimator->prp_isDescendantRecording()) {
        mPosAnimator->prp_startTransform();
    }
    mPivotAnimator->prp_startTransform();
}

void BoxTransformAnimator::pivotTransformFinished() {
    if(!mPosAnimator->prp_isDescendantRecording()) {
        mPosAnimator->prp_finishTransform();
    }
    mPivotAnimator->prp_finishTransform();
}

void BoxTransformAnimator::setPivotWithoutChangingTransformation(
                                const QPointF &point,
                                const bool &saveUndoRedo) {
    QMatrix currentMatrix;
    qreal pivotX = mPivotAnimator->getEffectiveXValue();
    qreal pivotY = mPivotAnimator->getEffectiveYValue();
    currentMatrix.translate(pivotX + mPosAnimator->getEffectiveXValue(),
                            pivotY + mPosAnimator->getEffectiveYValue());

    currentMatrix.rotate(mRotAnimator->qra_getCurrentEffectiveValue() );
    currentMatrix.scale(mScaleAnimator->getEffectiveXValue(),
                        mScaleAnimator->getEffectiveYValue() );

    currentMatrix.translate(-pivotX,
                            -pivotY);

    QMatrix futureMatrix;
    futureMatrix.translate(point.x() + mPosAnimator->getEffectiveXValue(),
                           point.y() + mPosAnimator->getEffectiveYValue());

    futureMatrix.rotate(mRotAnimator->qra_getCurrentEffectiveValue() );
    futureMatrix.scale(mScaleAnimator->getEffectiveXValue(),
                       mScaleAnimator->getEffectiveYValue() );

    futureMatrix.translate(-point.x(),
                           -point.y());


    mPosAnimator->incAllValues(currentMatrix.dx() - futureMatrix.dx(),
                               currentMatrix.dy() - futureMatrix.dy(),
                               saveUndoRedo,
                               false,
                               saveUndoRedo);

    mPivotAnimator->setCurrentPointValue(point,
                                         saveUndoRedo,
                                         false,
                                         saveUndoRedo);
}

QPointF BoxTransformAnimator::getPivot() {
    return mPivotAnimator->getCurrentEffectivePointValue();
}

QPointF BoxTransformAnimator::getPivotAbs() {
    return mPivotAnimator->getAbsolutePos();
}

qreal BoxTransformAnimator::getOpacityAtRelFrame(const int &relFrame) {
    return mOpacityAnimator->qra_getEffectiveValueAtRelFrame(relFrame);
}

bool BoxTransformAnimator::rotOrScaleOrPivotRecording() {
    return mRotAnimator->prp_isDescendantRecording() ||
           mScaleAnimator->prp_isDescendantRecording() ||
           mPivotAnimator->prp_isDescendantRecording();
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

    matrix.translate(-pivotX,
                     -pivotY);
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

    matrix.translate(-pivotX,
                     -pivotY);
    return matrix;
}

QMatrix BoxTransformAnimator::getCombinedTransformMatrixAtRelFrame(
        const int &relFrame) {
    if(mParentTransformAnimator.data() == NULL) {
        return mParentBox->getRelativeTransformAtRelFrame(relFrame);
    } else {
        int absFrame = prp_relFrameToAbsFrame(relFrame);
        int parentRelFrame =
                mParentTransformAnimator->prp_absFrameToRelFrame(absFrame);
        return mParentBox->getRelativeTransformAtRelFrame(relFrame)*
                mParentTransformAnimator->
                getCombinedTransformMatrixAtRelFrame(parentRelFrame);
    }
}

QMatrix BasicTransformAnimator::getParentCombinedTransformMatrixAtRelFrame(
        const int &relFrame) {
    if(mParentTransformAnimator.data() == NULL) {
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
    if(mParentTransformAnimator.data() == NULL) {
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

void BoxTransformAnimator::makeDuplicate(BoxTransformAnimator *target) {
    BasicTransformAnimator::makeDuplicate(target);
    target->duplicatePivotAnimatorFrom(mPivotAnimator.data());
    target->duplicateOpacityAnimatorFrom(mOpacityAnimator.data());
}

void BoxTransformAnimator::duplicatePivotAnimatorFrom(
        QPointFAnimator *source) {
    source->makeDuplicate(mPivotAnimator.data());
}

void BoxTransformAnimator::duplicateOpacityAnimatorFrom(
        QrealAnimator *source) {
    source->makeDuplicate(mOpacityAnimator.data());
}
