// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "MovablePoints/boxpathpoint.h"
#include "qrealanimator.h"
#include "transformanimator.h"
#include "qpointfanimator.h"
#include "MovablePoints/animatedpoint.h"
#include "skia/skqtconversions.h"
#include "matrixdecomposition.h"

BasicTransformAnimator::BasicTransformAnimator() :
    StaticComplexAnimator("transform") {
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

    connect(this, &Property::prp_currentFrameChanged,
            this, &BasicTransformAnimator::updateRelativeTransform);
}

void BasicTransformAnimator::resetScale() {
    mScaleAnimator->prp_startTransform();
    mScaleAnimator->setBaseValue(QPointF(1, 1));
    mScaleAnimator->prp_finishTransform();
}

void BasicTransformAnimator::resetTranslation() {
    mPosAnimator->prp_startTransform();
    mPosAnimator->setBaseValue(QPointF(0, 0));
    mPosAnimator->prp_finishTransform();
}

void BasicTransformAnimator::resetRotation() {
    mRotAnimator->prp_startTransform();
    mRotAnimator->setCurrentBaseValue(0);
    mRotAnimator->prp_finishTransform();
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
    if(!mParentTransform) return parentRelPos;
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
    if(!mParentTransform) return;
    const auto savedRelPos = mPosAnimator->getSavedValue();
    const auto savedAbsPos = mParentTransform->mapRelPosToAbs(savedRelPos);
    moveToAbs(savedAbsPos + absTrans);
}

void BasicTransformAnimator::moveToAbs(const QPointF &absPos) {
    setAbsolutePos(absPos);
}

void BasicTransformAnimator::setAbsolutePos(const QPointF &pos) {
    if(!mParentTransform) return;
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

void BasicTransformAnimator::updateInheritedTransform(const UpdateReason reason) {
    if(mParentTransform) {
        mInheritedTransform = mParentTransform->getTotalTransform();
    } else {
        mInheritedTransform.reset();
    }
    updateTotalTransform(reason);
}

void BasicTransformAnimator::updateTotalTransform(const UpdateReason reason) {
    if(mParentTransform) {
        mTotalTransform = mRelTransform*mInheritedTransform;
    } else {
        mTotalTransform = mRelTransform;
    }
    emit totalTransformChanged(reason);
}

const QMatrix &BasicTransformAnimator::getInheritedTransform() const {
    return mInheritedTransform;
}

const QMatrix &BasicTransformAnimator::getTotalTransform() const {
    return mTotalTransform;
}

const QMatrix &BasicTransformAnimator::getRelativeTransform() const {
    return mRelTransform;
}

void BasicTransformAnimator::setParentTransformAnimator(
        BasicTransformAnimator* parent) {
    auto& conn = mParentTransform.assign(parent);
    if(parent) {
        conn << connect(parent, &BasicTransformAnimator::totalTransformChanged,
                        this, &BasicTransformAnimator::updateInheritedTransform);
    }
    updateInheritedTransform(UpdateReason::userChange);
}

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

QPointFAnimator *BasicTransformAnimator::getPosAnimator() const {
    return mPosAnimator.get();
}

QPointFAnimator *BasicTransformAnimator::getScaleAnimator() const {
    return mScaleAnimator.get();
}

QrealAnimator *BasicTransformAnimator::getRotAnimator() const {
    return mRotAnimator.get();
}

QMatrix BasicTransformAnimator::getInheritedTransformAtFrame(
        const qreal relFrame) {
    if(mParentTransform) {
        const qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
        const qreal parentRelFrame =
                mParentTransform->prp_absFrameToRelFrameF(absFrame);
        return mParentTransform->getTotalTransformAtFrame(parentRelFrame);
    } else {
        return QMatrix();
    }
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

AdvancedTransformAnimator::AdvancedTransformAnimator() {
    mShearAnimator = enve::make_shared<QPointFAnimator>("shear");
    mShearAnimator->setBaseValue(QPointF(0, 0));
    mShearAnimator->setValuesRange(-100, 100);
    mShearAnimator->setPrefferedValueStep(0.1);

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
}

void AdvancedTransformAnimator::resetShear() {
    mShearAnimator->setBaseValue(QPointF(0, 0));
}

void AdvancedTransformAnimator::resetPivot() {
    mPivotAnimator->setBaseValue(QPointF(0, 0));
}

void AdvancedTransformAnimator::resetRotScaleShear() {
    resetRotation();
    resetScale();
    resetShear();
}

void AdvancedTransformAnimator::reset() {
    BasicTransformAnimator::reset();
    resetShear();
    resetPivot();
}

void AdvancedTransformAnimator::startOpacityTransform() {
    mOpacityAnimator->prp_startTransform();
}

void AdvancedTransformAnimator::setOpacity(const qreal newOpacity) {
    mOpacityAnimator->setCurrentBaseValue(newOpacity);
}

void AdvancedTransformAnimator::setPivot(const qreal x, const qreal y) {
    mPivotAnimator->setBaseValue(QPointF(x, y));
}

void AdvancedTransformAnimator::startPivotTransform() {
    if(!mPosAnimator->anim_isDescendantRecording())
        mPosAnimator->prp_startTransform();
    mPivotAnimator->prp_startTransform();
}

void AdvancedTransformAnimator::finishPivotTransform() {
    if(!mPosAnimator->anim_isDescendantRecording())
        mPosAnimator->prp_finishTransform();
    mPivotAnimator->prp_finishTransform();
}

void AdvancedTransformAnimator::setPivotFixedTransform(
        const QPointF &newPivot) {
    TransformValues oldTransfom;
    oldTransfom.fPivotX = mPivotAnimator->getEffectiveXValue();
    oldTransfom.fPivotY = mPivotAnimator->getEffectiveYValue();
    oldTransfom.fMoveX = mPosAnimator->getEffectiveXValue();
    oldTransfom.fMoveY = mPosAnimator->getEffectiveYValue();
    oldTransfom.fRotation = mRotAnimator->getEffectiveValue();
    oldTransfom.fScaleX = mScaleAnimator->getEffectiveXValue();
    oldTransfom.fScaleY = mScaleAnimator->getEffectiveYValue();
    oldTransfom.fShearX = mShearAnimator->getEffectiveXValue();
    oldTransfom.fShearY = mShearAnimator->getEffectiveYValue();
    const auto newTransform = MatrixDecomposition::
            setPivotKeepTransform(oldTransfom, newPivot);

    const qreal posXInc = newTransform.fMoveX - oldTransfom.fMoveX;
    const qreal posYInc = newTransform.fMoveY - oldTransfom.fMoveY;
    const bool posAnimated = mPosAnimator->anim_isDescendantRecording();
    const bool pivotAnimated = mPivotAnimator->anim_isDescendantRecording();
    if(pivotAnimated) {
        mPivotAnimator->setBaseValue(newPivot);
    } else if(posAnimated && !pivotAnimated) {
        mPosAnimator->incAllBaseValues(posXInc, posYInc);
        mPivotAnimator->setBaseValueWithoutCallingUpdater(newPivot);
    } else { // if(!posAnimated && !pivotAnimated) {
        mPosAnimator->incBaseValuesWithoutCallingUpdater(posXInc, posYInc);
        mPivotAnimator->setBaseValueWithoutCallingUpdater(newPivot);
    }
}

QPointF AdvancedTransformAnimator::getPivot() {
    return mPivotAnimator->getEffectiveValue();
}

QPointF AdvancedTransformAnimator::getPivotAbs() {
    return mapRelPosToAbs(mPivotAnimator->getEffectiveValue());
}

qreal AdvancedTransformAnimator::getOpacity(const qreal relFrame) {
    return mOpacityAnimator->getEffectiveValue(relFrame);
}

bool AdvancedTransformAnimator::posOrPivotRecording() const {
    return mPosAnimator->anim_isDescendantRecording() ||
           mPivotAnimator->anim_isDescendantRecording();
}

bool AdvancedTransformAnimator::rotOrScaleOrPivotRecording() const {
    return mRotAnimator->anim_isDescendantRecording() ||
           mScaleAnimator->anim_isDescendantRecording() ||
           mPivotAnimator->anim_isDescendantRecording();
}

qreal AdvancedTransformAnimator::getPivotX() {
    return mPivotAnimator->getEffectiveXValue();
}

qreal AdvancedTransformAnimator::getPivotY() {
    return mPivotAnimator->getEffectiveYValue();
}

void AdvancedTransformAnimator::startRotScaleShearTransform() {
    startRotTransform();
    startScaleTransform();
    startShearTransform();
}

void AdvancedTransformAnimator::startShearTransform() {
    mShearAnimator->prp_startTransform();
}

void AdvancedTransformAnimator::setShear(const qreal shearX, const qreal shearY) {
    mShearAnimator->setBaseValue(shearX, shearY);
}

qreal AdvancedTransformAnimator::getOpacity() {
    return mOpacityAnimator->getEffectiveValue();
}

void AdvancedTransformAnimator::startTransformSkipOpacity() {
    startPosTransform();
    startPivotTransform();
    startRotTransform();
    startScaleTransform();
    startShearTransform();
}

QMatrix AdvancedTransformAnimator::getCurrentTransform() {
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

void AdvancedTransformAnimator::setValues(const TransformValues &values) {
    setPivot(values.fPivotX, values.fPivotY);
    setPosition(values.fMoveX, values.fMoveY);
    setScale(values.fScaleX, values.fScaleY);
    setRotation(values.fRotation);
    setShear(values.fShearX, values.fShearY);
}

QMatrix AdvancedTransformAnimator::getRotScaleShearTransform() {
    const qreal pivotX = mPivotAnimator->getEffectiveXValue();
    const qreal pivotY = mPivotAnimator->getEffectiveYValue();

    QMatrix matrix;
    matrix.translate(pivotX, pivotY);
    matrix.rotate(mRotAnimator->getEffectiveValue());
    matrix.scale(mScaleAnimator->getEffectiveXValue(),
                 mScaleAnimator->getEffectiveYValue());
    matrix.shear(mShearAnimator->getEffectiveXValue(),
                 mShearAnimator->getEffectiveYValue());
    matrix.translate(-pivotX, -pivotY);
    return matrix;
}

QMatrix AdvancedTransformAnimator::getRelativeTransformAtFrame(const qreal relFrame) {
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

BoxTransformAnimator::BoxTransformAnimator() {
    setPointsHandler(enve::make_shared<PointsHandler>());
    const auto pivotPt = enve::make_shared<BoxPathPoint>(
                getPivotAnimator(), this);
    getPointsHandler()->appendPt(pivotPt);
}

void BoxTransformAnimator::saveSVG(QDomDocument& doc,
                                   QDomElement& parent,
                                   QDomElement& defs,
                                   const FrameRange& absRange,
                                   const qreal fps,
                                   const QDomElement& child) const {
    auto pivot = doc.createElement("g");
    auto translate = doc.createElement("g");
    auto rotate = doc.createElement("g");
    auto scale = doc.createElement("g");
    auto shear = doc.createElement("g");
    auto unpivot = doc.createElement("g");

    const auto pivotAnim = getPivotAnimator();
    pivotAnim->saveSVG(doc, pivot, defs, absRange, fps,
                       "transform", true, "translate");
    const auto opacityAnim = getOpacityAnimator();
    opacityAnim->Animator::saveSVG(doc, pivot, defs, absRange, fps, "opacity",
                                   [opacityAnim](const int relFrame) {
        const qreal value = opacityAnim->getEffectiveValue(relFrame);
        return QString::number(value/100);
    });
    getPosAnimator()->saveSVG(doc, translate, defs, absRange, fps,
                              "transform", true, "translate");
    getRotAnimator()->saveSVG(doc, rotate, defs, absRange, fps,
                              "transform", true, "rotate");
    getScaleAnimator()->saveSVG(doc, scale, defs, absRange, fps,
                                "transform", true, "scale");
    const auto shearAnim = getShearAnimator();
    const auto shearXAnim = shearAnim->getXAnimator();
    const auto shearYAnim = shearAnim->getYAnimator();
    shearXAnim->Animator::saveSVG(doc, shear, defs, absRange, fps, "transform",
                                  [shearXAnim](const int relFrame) {
        const qreal value = shearXAnim->getEffectiveValue(relFrame);
        return QString::number(value*45);
    }, true, "skewX");
    shearYAnim->Animator::saveSVG(doc, shear, defs, absRange, fps, "transform",
                                  [shearYAnim](const int relFrame) {
        const qreal value = shearYAnim->getEffectiveValue(relFrame);
        return QString::number(value*45);
    }, true, "skewY");

    const auto animGetter = [pivotAnim](const int relFrame) {
        const auto value = pivotAnim->getEffectiveValue(relFrame);
        return QString::number(-value.x()) + " " +
               QString::number(-value.y());
    };
    pivotAnim->Animator::saveSVG(doc, unpivot, defs, absRange, fps,
                                 "transform", animGetter, true, "translate");

    unpivot.appendChild(child);
    shear.appendChild(unpivot);
    scale.appendChild(shear);
    rotate.appendChild(scale);
    translate.appendChild(rotate);
    pivot.appendChild(translate);

    parent.appendChild(pivot);
}
