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
#include "PropertyUpdaters/boxpathpointupdater.h"

BasicTransformAnimator::BasicTransformAnimator() :
    ComplexAnimator("transformation") {
    mPosAnimator = SPtrCreate(QPointFAnimator)("pos");
    mPosAnimator->setBaseValue(QPointF(0, 0));

    mScaleAnimator = SPtrCreate(QPointFAnimator)("scale");
    mScaleAnimator->setBaseValue(QPointF(1, 1));
    mScaleAnimator->setPrefferedValueStep(0.05);

    mRotAnimator = SPtrCreate(QrealAnimator)("rot");
    mRotAnimator->setCurrentBaseValue(0);

    ca_addChildAnimator(mPosAnimator);
    ca_addChildAnimator(mRotAnimator);
    ca_addChildAnimator(mScaleAnimator);
}

void BasicTransformAnimator::resetScale() {
    mScaleAnimator->setBaseValue(QPointF(1, 1));
}

void BasicTransformAnimator::resetTranslation() {
    mPosAnimator->setBaseValue(QPointF(0, 0));
}

void BasicTransformAnimator::resetRotation() {
    mRotAnimator->setCurrentBaseValue(0);
}

void BasicTransformAnimator::reset() {
    resetScale();
    resetTranslation();
    resetRotation();
}

void BasicTransformAnimator::setScale(const qreal &sx, const qreal &sy) {
    mScaleAnimator->setBaseValue(QPointF(sx, sy));
}

void BasicTransformAnimator::setPosition(const qreal &x, const qreal &y) {
    mPosAnimator->setBaseValue(QPointF(x, y));
}

void BasicTransformAnimator::setRotation(const qreal &rot) {
    mRotAnimator->setCurrentBaseValue(rot);
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

void BasicTransformAnimator::moveRelativeToSavedValue(const qreal &dX, const qreal &dY) {
    mPosAnimator->incSavedValueToCurrentValue(dX, dY);
}

void BasicTransformAnimator::translate(const qreal &dX, const qreal &dY) {
    mPosAnimator->incBaseValues(dX, dY);
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
    return mRotAnimator->getCurrentEffectiveValue();
}

qreal BasicTransformAnimator::xScale() {
    return mScaleAnimator->getEffectiveXValue();
}

qreal BasicTransformAnimator::yScale() {
    return mScaleAnimator->getEffectiveYValue();
}

QPointF BasicTransformAnimator::pos() {
    return mPosAnimator->getEffectiveValue();
}

QPointF BasicTransformAnimator::mapAbsPosToRel(const QPointF &absPos) const {
    return getTotalTransform().inverted().map(absPos);
}

QPointF BasicTransformAnimator::mapRelPosToAbs(const QPointF &relPos) const {
    return getTotalTransform().map(relPos);
}

QPointF BasicTransformAnimator::mapFromParent(const QPointF &parentRelPos) const {
    const auto absPos = mParentTransformAnimator->mapRelPosToAbs(parentRelPos);
    return mapAbsPosToRel(absPos);
}

SkPoint BasicTransformAnimator::mapAbsPosToRel(const SkPoint &absPos) const {
    return toSkPoint(mapAbsPosToRel(toQPointF(absPos)));
}

SkPoint BasicTransformAnimator::mapRelPosToAbs(const SkPoint &relPos) const {
    return toSkPoint(mapRelPosToAbs(toQPointF(relPos)));
}

SkPoint BasicTransformAnimator::mapFromParent(const SkPoint &parentRelPos) const {
    return toSkPoint(mapFromParent(toQPointF(parentRelPos)));
}

QMatrix BasicTransformAnimator::getCurrentTransformationMatrix() {
    QMatrix matrix;
    matrix.translate(mPosAnimator->getEffectiveXValue(),
                     mPosAnimator->getEffectiveYValue());

    matrix.rotate(mRotAnimator->getCurrentEffectiveValue());
    matrix.scale(mScaleAnimator->getEffectiveXValue(),
                 mScaleAnimator->getEffectiveYValue());
    return matrix;
}

QMatrix BasicTransformAnimator::getRelativeTransformAtRelFrame(
                                    const qreal &relFrame) {
    QMatrix matrix;
    matrix.translate(mPosAnimator->getEffectiveXValueAtRelFrame(relFrame),
                     mPosAnimator->getEffectiveYValueAtRelFrame(relFrame));

    matrix.rotate(mRotAnimator->getEffectiveValueAtRelFrame(relFrame));
    matrix.scale(mScaleAnimator->getEffectiveXValueAtRelFrame(relFrame),
                 mScaleAnimator->getEffectiveYValueAtRelFrame(relFrame));
    return matrix;
}

void BasicTransformAnimator::moveByAbs(const QPointF &absTrans) {
    const auto savedRelPos = mPosAnimator->getSavedValue();
    const auto savedAbsPos = mParentTransformAnimator->mapRelPosToAbs(savedRelPos);
    moveToAbs(savedAbsPos + absTrans);
}

void BasicTransformAnimator::moveToAbs(const QPointF &absPos) {
    setAbsolutePos(absPos);
}

void BasicTransformAnimator::setAbsolutePos(const QPointF &pos) {
    setRelativePos(mParentTransformAnimator->mapAbsPosToRel(pos));
}

void BasicTransformAnimator::setRelativePos(const QPointF &relPos) {
    mPosAnimator->setBaseValue(relPos);
}

void BasicTransformAnimator::rotateRelativeToSavedValue(const qreal &rotRel,
                                                        const QPointF &pivot) {
    QMatrix matrix;
    matrix.translate(pivot.x(), pivot.y());
    matrix.rotate(rotRel);
    matrix.translate(-pivot.x() + mPosAnimator->getSavedXValue(),
                     -pivot.y() + mPosAnimator->getSavedYValue());
    rotateRelativeToSavedValue(rotRel);
    mPosAnimator->setBaseValue(QPointF(matrix.dx(), matrix.dy()));
}

void BasicTransformAnimator::updateRelativeTransform(const UpdateReason &reason) {
    mRelTransform = getCurrentTransformationMatrix();
    updateTotalTransform(reason);
}

void BasicTransformAnimator::updateTotalTransform(const UpdateReason &reason) {
    if(mParentTransformAnimator.isNull()) {
        mTotalTransform = mRelTransform;
    } else {
        mTotalTransform = mRelTransform *
                             mParentTransformAnimator->getTotalTransform();
    }
    emit TotalTransformChanged(reason);
}

const QMatrix &BasicTransformAnimator::getTotalTransform() const {
    return mTotalTransform;
}

const QMatrix &BasicTransformAnimator::getRelativeTransform() const {
    return mRelTransform;
}

void BasicTransformAnimator::setParentTransformAnimator(
        BasicTransformAnimator* parent) {
    if(mParentTransformAnimator)
        disconnect(mParentTransformAnimator,
                   &BasicTransformAnimator::TotalTransformChanged,
                   this, &BasicTransformAnimator::updateTotalTransform);
    mParentTransformAnimator = parent;
    if(parent) connect(parent, &BasicTransformAnimator::TotalTransformChanged,
                       this, &BasicTransformAnimator::updateTotalTransform);
    updateTotalTransform(Animator::USER_CHANGE);
}

bool BasicTransformAnimator::SWT_isBasicTransformAnimator() const { return true; }

void BasicTransformAnimator::scaleRelativeToSavedValue(const qreal &sx,
                                                       const qreal &sy,
                                                      const QPointF &pivot) {
    QMatrix matrix;
    matrix.translate(pivot.x(), pivot.y());
    matrix.rotate(mRotAnimator->getCurrentEffectiveValue());
    matrix.scale(sx, sy);
    matrix.rotate(-mRotAnimator->getCurrentEffectiveValue());
    matrix.translate(-pivot.x() + mPosAnimator->getSavedXValue(),
                     -pivot.y() + mPosAnimator->getSavedYValue());

    scale(sx, sy);
    mPosAnimator->setBaseValue(QPointF(matrix.dx(), matrix.dy()));
}

BoxTransformAnimator::BoxTransformAnimator(BoundingBox * const parent) :
    BasicTransformAnimator(), mParentBox_k(parent) {
    mPivotAnimator = SPtrCreate(QPointFAnimator)("pivot");
    mPivotAnimator->setBaseValue(QPointF(0, 0));

    mOpacityAnimator = SPtrCreate(QrealAnimator)("opacity");
    mOpacityAnimator->setValueRange(0, 100);
    mOpacityAnimator->setPrefferedValueStep(5);
    mOpacityAnimator->setCurrentBaseValue(100);
    mOpacityAnimator->graphFixMinMaxValues();

    mPivotPoint = SPtrCreate(BoxPathPoint)(mPivotAnimator.get(), this);

    ca_addChildAnimator(mPivotAnimator);
    ca_addChildAnimator(mOpacityAnimator);
}

MovablePoint *BoxTransformAnimator::getPivotMovablePoint() {
    return mPivotPoint.get();
}

void BoxTransformAnimator::resetPivot() {
    mPivotAnimator->setBaseValue(QPointF(0, 0));
}

void BoxTransformAnimator::reset() {
    BasicTransformAnimator::reset();
    resetPivot();
}

void BoxTransformAnimator::startOpacityTransform() {
    mOpacityAnimator->prp_startTransform();
}

void BoxTransformAnimator::setOpacity(const qreal &newOpacity) {
    mOpacityAnimator->setCurrentBaseValue(newOpacity);
}

void BoxTransformAnimator::startPivotTransform() {
    if(!mPosAnimator->anim_isDescendantRecording())
        mPosAnimator->prp_startTransform();
    mPivotAnimator->prp_startTransform();
}

void BoxTransformAnimator::finishPivotTransform() {
    if(!mPosAnimator->anim_isDescendantRecording())
        mPosAnimator->prp_finishTransform();
    mPivotAnimator->prp_finishTransform();
}

void BoxTransformAnimator::setPivotWithoutChangingTransformation(const QPointF &point) {
    QMatrix currentMatrix;
    const qreal pivotX = mPivotAnimator->getEffectiveXValue();
    const qreal pivotY = mPivotAnimator->getEffectiveYValue();
    currentMatrix.translate(pivotX + mPosAnimator->getEffectiveXValue(),
                            pivotY + mPosAnimator->getEffectiveYValue());

    currentMatrix.rotate(mRotAnimator->getCurrentEffectiveValue());
    currentMatrix.scale(mScaleAnimator->getEffectiveXValue(),
                        mScaleAnimator->getEffectiveYValue());

    currentMatrix.translate(-pivotX, -pivotY);

    QMatrix futureMatrix;
    futureMatrix.translate(point.x() + mPosAnimator->getEffectiveXValue(),
                           point.y() + mPosAnimator->getEffectiveYValue());

    futureMatrix.rotate(mRotAnimator->getCurrentEffectiveValue());
    futureMatrix.scale(mScaleAnimator->getEffectiveXValue(),
                       mScaleAnimator->getEffectiveYValue());

    futureMatrix.translate(-point.x(), -point.y());

    const qreal posXInc = currentMatrix.dx() - futureMatrix.dx();
    const qreal posYInc = currentMatrix.dy() - futureMatrix.dy();
    if(posOrPivotRecording()) {
        mPosAnimator->incAllBaseValues(posXInc, posYInc);
        mPivotAnimator->setBaseValue(point);
    } else {
        mPosAnimator->incBaseValuesWithoutCallingUpdater(posXInc, posYInc);
        mPivotAnimator->setBaseValueWithoutCallingUpdater(point);
    }
}

QPointF BoxTransformAnimator::getPivot() {
    return mPivotAnimator->getEffectiveValue();
}

QPointF BoxTransformAnimator::getPivotAbs() {
    return mPivotPoint->getAbsolutePos();
}

qreal BoxTransformAnimator::getOpacityAtRelFrameF(const qreal &relFrame) {
    return mOpacityAnimator->getEffectiveValueAtRelFrame(relFrame);
}

bool BoxTransformAnimator::posOrPivotRecording() const {
    return mPosAnimator->anim_isDescendantRecording() ||
           mPivotAnimator->anim_isDescendantRecording();
}

bool BoxTransformAnimator::rotOrScaleOrPivotRecording() const {
    return mRotAnimator->anim_isDescendantRecording() ||
           mScaleAnimator->anim_isDescendantRecording() ||
           mPivotAnimator->anim_isDescendantRecording();
}

void BoxTransformAnimator::updateTotalTransform(const UpdateReason &reason) {
    BasicTransformAnimator::updateTotalTransform(reason);

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
    return mOpacityAnimator->getCurrentBaseValue();
}

QMatrix BoxTransformAnimator::getCurrentTransformationMatrix() {
    const qreal pivotX = mPivotAnimator->getEffectiveXValue();
    const qreal pivotY = mPivotAnimator->getEffectiveYValue();
    QMatrix matrix;
    matrix.translate(pivotX + mPosAnimator->getEffectiveXValue(),
                     pivotY + mPosAnimator->getEffectiveYValue());

    matrix.rotate(mRotAnimator->getCurrentEffectiveValue());
    matrix.scale(mScaleAnimator->getEffectiveXValue(),
                 mScaleAnimator->getEffectiveYValue());

    matrix.translate(-pivotX, -pivotY);
    return matrix;
}

QMatrix BoxTransformAnimator::getRelativeTransformAtRelFrame(
                                    const qreal &relFrame) {
    const qreal pivotX = mPivotAnimator->getEffectiveXValueAtRelFrame(relFrame);
    const qreal pivotY = mPivotAnimator->getEffectiveYValueAtRelFrame(relFrame);
    QMatrix matrix;
    matrix.translate(pivotX + mPosAnimator->getEffectiveXValueAtRelFrame(relFrame),
                     pivotY + mPosAnimator->getEffectiveYValueAtRelFrame(relFrame));

    matrix.rotate(mRotAnimator->getEffectiveValueAtRelFrame(relFrame));
    matrix.scale(mScaleAnimator->getEffectiveXValueAtRelFrame(relFrame),
                 mScaleAnimator->getEffectiveYValueAtRelFrame(relFrame));

    matrix.translate(-pivotX, -pivotY);
    return matrix;
}

QMatrix BasicTransformAnimator::getParentTotalTransformAtRelFrame(
        const qreal &relFrame) {
    if(mParentTransformAnimator.data() == nullptr) {
        return QMatrix();
    } else {
        const qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
        const qreal parentRelFrame =
                mParentTransformAnimator->prp_absFrameToRelFrameF(absFrame);
        return mParentTransformAnimator->
                getTotalTransformAtRelFrameF(parentRelFrame);
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

QMatrix BasicTransformAnimator::getTotalTransformAtRelFrameF(
        const qreal &relFrame) {
    if(mParentTransformAnimator.data()) {
        const qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
        const qreal parentRelFrame =
                mParentTransformAnimator->prp_absFrameToRelFrameF(absFrame);
        return getRelativeTransformAtRelFrame(relFrame)*
                mParentTransformAnimator->
                    getTotalTransformAtRelFrameF(parentRelFrame);
    } else {
        return getRelativeTransformAtRelFrame(relFrame);
    }
}
