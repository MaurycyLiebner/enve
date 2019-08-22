#include "MovablePoints/boxpathpoint.h"
#include "PropertyUpdaters/transformupdater.h"
#include "qrealanimator.h"
#include "transformanimator.h"
#include "qpointfanimator.h"
#include "MovablePoints/animatedpoint.h"
#include "skia/skqtconversions.h"

BasicTransformAnimator::BasicTransformAnimator() :
    StaticComplexAnimator("transformation") {
    mPosAnimator = enve::make_shared<QPointFAnimator>("pos");
    mPosAnimator->setBaseValue(QPointF(0, 0));

    mScaleAnimator = enve::make_shared<QPointFAnimator>("scale");
    mScaleAnimator->setBaseValue(QPointF(1, 1));
    mScaleAnimator->setPrefferedValueStep(0.05);

    mRotAnimator = enve::make_shared<QrealAnimator>("rot");
    mRotAnimator->setCurrentBaseValue(0);

    ca_addChild(mPosAnimator);
    ca_addChild(mRotAnimator);
    ca_addChild(mScaleAnimator);
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

void BasicTransformAnimator::setScale(const qreal sx, const qreal sy) {
    mScaleAnimator->setBaseValue(QPointF(sx, sy));
}

void BasicTransformAnimator::setPosition(const qreal x, const qreal y) {
    mPosAnimator->setBaseValue(QPointF(x, y));
}

void BasicTransformAnimator::setRotation(const qreal rot) {
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

void BasicTransformAnimator::rotateRelativeToSavedValue(const qreal rotRel) {
    mRotAnimator->incSavedValueToCurrentValue(rotRel);
}

void BasicTransformAnimator::moveRelativeToSavedValue(const qreal dX, const qreal dY) {
    mPosAnimator->incSavedValueToCurrentValue(dX, dY);
}

void BasicTransformAnimator::translate(const qreal dX, const qreal dY) {
    mPosAnimator->incBaseValues(dX, dY);
}

void BasicTransformAnimator::scale(const qreal sx, const qreal sy) {
    mScaleAnimator->multSavedValueToCurrentValue(sx, sy);
}

qreal BasicTransformAnimator::dx() {
    return mPosAnimator->getEffectiveXValue();
}

qreal BasicTransformAnimator::dy() {
    return mPosAnimator->getEffectiveYValue();
}

qreal BasicTransformAnimator::rot() {
    return mRotAnimator->getEffectiveValue();
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
    const auto absPos = mParentTransform->mapRelPosToAbs(parentRelPos);
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

QMatrix BasicTransformAnimator::getCurrentTransform() {
    QMatrix matrix;
    matrix.translate(mPosAnimator->getEffectiveXValue(),
                     mPosAnimator->getEffectiveYValue());

    matrix.rotate(mRotAnimator->getEffectiveValue());
    matrix.scale(mScaleAnimator->getEffectiveXValue(),
                 mScaleAnimator->getEffectiveYValue());
    return matrix;
}

QMatrix BasicTransformAnimator::getRelativeTransformAtFrame(const qreal relFrame) {
    QMatrix matrix;
    matrix.translate(mPosAnimator->getEffectiveXValue(relFrame),
                     mPosAnimator->getEffectiveYValue(relFrame));

    matrix.rotate(mRotAnimator->getEffectiveValue(relFrame));
    matrix.scale(mScaleAnimator->getEffectiveXValue(relFrame),
                 mScaleAnimator->getEffectiveYValue(relFrame));
    return matrix;
}

void BasicTransformAnimator::moveByAbs(const QPointF &absTrans) {
    const auto savedRelPos = mPosAnimator->getSavedValue();
    const auto savedAbsPos = mParentTransform->mapRelPosToAbs(savedRelPos);
    moveToAbs(savedAbsPos + absTrans);
}

void BasicTransformAnimator::moveToAbs(const QPointF &absPos) {
    setAbsolutePos(absPos);
}

void BasicTransformAnimator::setAbsolutePos(const QPointF &pos) {
    setRelativePos(mParentTransform->mapAbsPosToRel(pos));
}

void BasicTransformAnimator::setRelativePos(const QPointF &relPos) {
    mPosAnimator->setBaseValue(relPos);
}

void BasicTransformAnimator::rotateRelativeToSavedValue(const qreal rotRel,
                                                        const QPointF &pivot) {
    QMatrix matrix;
    matrix.translate(pivot.x(), pivot.y());
    matrix.rotate(rotRel);
    matrix.translate(-pivot.x() + mPosAnimator->getSavedXValue(),
                     -pivot.y() + mPosAnimator->getSavedYValue());
    rotateRelativeToSavedValue(rotRel);
    mPosAnimator->setBaseValue(QPointF(matrix.dx(), matrix.dy()));
}

void BasicTransformAnimator::updateRelativeTransform(const UpdateReason reason) {
    mRelTransform = getCurrentTransform();
    updateTotalTransform(reason);
}

void BasicTransformAnimator::updateTotalTransform(const UpdateReason reason) {
    if(mParentTransform.isNull()) {
        mTotalTransform = mRelTransform;
    } else {
        mTotalTransform = mRelTransform *
                             mParentTransform->getTotalTransform();
    }
    emit totalTransformChanged(reason);
}

const QMatrix &BasicTransformAnimator::getTotalTransform() const {
    return mTotalTransform;
}

const QMatrix &BasicTransformAnimator::getRelativeTransform() const {
    return mRelTransform;
}

void BasicTransformAnimator::setParentTransformAnimator(
        BasicTransformAnimator* parent) {
    if(mParentTransform)
        disconnect(mParentTransform,
                   &BasicTransformAnimator::totalTransformChanged,
                   this, &BasicTransformAnimator::updateTotalTransform);
    mParentTransform = parent;
    if(parent) connect(parent, &BasicTransformAnimator::totalTransformChanged,
                       this, &BasicTransformAnimator::updateTotalTransform);
    updateTotalTransform(UpdateReason::userChange);
}

bool BasicTransformAnimator::SWT_isBasicTransformAnimator() const { return true; }

void BasicTransformAnimator::scaleRelativeToSavedValue(const qreal sx,
                                                       const qreal sy,
                                                      const QPointF &pivot) {
    QMatrix matrix;
    matrix.translate(pivot.x(), pivot.y());
    matrix.rotate(mRotAnimator->getEffectiveValue());
    matrix.scale(sx, sy);
    matrix.rotate(-mRotAnimator->getEffectiveValue());
    matrix.translate(-pivot.x() + mPosAnimator->getSavedXValue(),
                     -pivot.y() + mPosAnimator->getSavedYValue());

    scale(sx, sy);
    mPosAnimator->setBaseValue(QPointF(matrix.dx(), matrix.dy()));
}

QMatrix BasicTransformAnimator::getParentTotalTransformAtRelFrame(
        const qreal relFrame) {
    if(mParentTransform.data() == nullptr) {
        return QMatrix();
    } else {
        const qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
        const qreal parentRelFrame =
                mParentTransform->prp_absFrameToRelFrameF(absFrame);
        return mParentTransform->
                getTotalTransformAtFrame(parentRelFrame);
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

QMatrix BasicTransformAnimator::getTotalTransformAtFrame(
        const qreal relFrame) {
    if(mParentTransform) {
        const qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
        const qreal parentRelFrame =
                mParentTransform->prp_absFrameToRelFrameF(absFrame);
        return getRelativeTransformAtFrame(relFrame)*
                mParentTransform->getTotalTransformAtFrame(parentRelFrame);
    } else {
        return getRelativeTransformAtFrame(relFrame);
    }
}

BoxTransformAnimator::BoxTransformAnimator() {
    mShearAnimator = enve::make_shared<QPointFAnimator>("shear");
    mShearAnimator->setBaseValue(QPointF(0, 0));
    mShearAnimator->setValuesRange(-1, 1);

    mPivotAnimator = enve::make_shared<QPointFAnimator>("pivot");
    mPivotAnimator->setBaseValue(QPointF(0, 0));

    mOpacityAnimator = enve::make_shared<QrealAnimator>("opacity");
    mOpacityAnimator->setValueRange(0, 100);
    mOpacityAnimator->setPrefferedValueStep(5);
    mOpacityAnimator->setCurrentBaseValue(100);
    mOpacityAnimator->graphFixMinMaxValues();

    ca_addChild(mShearAnimator);
    ca_addChild(mPivotAnimator);
    ca_addChild(mOpacityAnimator);

    setPointsHandler(enve::make_shared<PointsHandler>());
    const auto pivotPt = enve::make_shared<BoxPathPoint>(mPivotAnimator.get(), this);
    mPointsHandler->appendPt(pivotPt);
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

void BoxTransformAnimator::setOpacity(const qreal newOpacity) {
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

void BoxTransformAnimator::setPivotFixedTransform(
        const QPointF &point) {
    const QMatrix currentMatrix = getCurrentTransform();
    QMatrix futureMatrix;
    futureMatrix.translate(point.x() + mPosAnimator->getEffectiveXValue(),
                           point.y() + mPosAnimator->getEffectiveYValue());

    futureMatrix.rotate(mRotAnimator->getEffectiveValue());
    futureMatrix.scale(mScaleAnimator->getEffectiveXValue(),
                       mScaleAnimator->getEffectiveYValue());
    futureMatrix.shear(mShearAnimator->getEffectiveXValue(),
                       mShearAnimator->getEffectiveYValue());

    futureMatrix.translate(-point.x(), -point.y());

    const qreal posXInc = currentMatrix.dx() - futureMatrix.dx();
    const qreal posYInc = currentMatrix.dy() - futureMatrix.dy();
    const bool posAnimated = mPosAnimator->anim_isDescendantRecording();
    const bool pivotAnimated = mPivotAnimator->anim_isDescendantRecording();
    if(pivotAnimated) {
        mPivotAnimator->setBaseValue(point);
    } else if(posAnimated && !pivotAnimated) {
        mPosAnimator->incAllBaseValues(posXInc, posYInc);
        mPivotAnimator->setBaseValueWithoutCallingUpdater(point);
    } else { // if(!posAnimated && !pivotAnimated) {
        mPosAnimator->incBaseValuesWithoutCallingUpdater(posXInc, posYInc);
        mPivotAnimator->setBaseValueWithoutCallingUpdater(point);
    }
}

QPointF BoxTransformAnimator::getPivot() {
    return mPivotAnimator->getEffectiveValue();
}

QPointF BoxTransformAnimator::getPivotAbs() {
    return mapRelPosToAbs(mPivotAnimator->getEffectiveValue());
}

qreal BoxTransformAnimator::getOpacity(const qreal relFrame) {
    return mOpacityAnimator->getEffectiveValue(relFrame);
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

qreal BoxTransformAnimator::getPivotX() {
    return mPivotAnimator->getEffectiveXValue();
}

qreal BoxTransformAnimator::getPivotY() {
    return mPivotAnimator->getEffectiveYValue();
}

void BoxTransformAnimator::setShear(const qreal shearX, const qreal shearY) {
    mShearAnimator->setBaseValue(shearX, shearY);
}

qreal BoxTransformAnimator::getOpacity() {
    return mOpacityAnimator->getCurrentBaseValue();
}

QMatrix BoxTransformAnimator::getCurrentTransform() {
    const qreal pivotX = mPivotAnimator->getEffectiveXValue();
    const qreal pivotY = mPivotAnimator->getEffectiveYValue();
    QMatrix matrix;
    matrix.translate(pivotX + mPosAnimator->getEffectiveXValue(),
                     pivotY + mPosAnimator->getEffectiveYValue());

    matrix.rotate(mRotAnimator->getEffectiveValue());
    matrix.scale(mScaleAnimator->getEffectiveXValue(),
                 mScaleAnimator->getEffectiveYValue());
    matrix.shear(mShearAnimator->getEffectiveXValue(),
                 mShearAnimator->getEffectiveYValue());

    matrix.translate(-pivotX, -pivotY);
    return matrix;
}

QMatrix BoxTransformAnimator::getRelativeTransformAtFrame(const qreal relFrame) {
    const qreal pivotX = mPivotAnimator->getEffectiveXValue(relFrame);
    const qreal pivotY = mPivotAnimator->getEffectiveYValue(relFrame);
    QMatrix matrix;
    matrix.translate(pivotX + mPosAnimator->getEffectiveXValue(relFrame),
                     pivotY + mPosAnimator->getEffectiveYValue(relFrame));

    matrix.rotate(mRotAnimator->getEffectiveValue(relFrame));
    matrix.scale(mScaleAnimator->getEffectiveXValue(relFrame),
                 mScaleAnimator->getEffectiveYValue(relFrame));
    matrix.shear(mShearAnimator->getEffectiveXValue(relFrame),
                 mShearAnimator->getEffectiveYValue(relFrame));

    matrix.translate(-pivotX, -pivotY);
    return matrix;
}
