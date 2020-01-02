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

#include "qpointfanimator.h"
#include "qrealanimator.h"

QPointFAnimator::QPointFAnimator(const QString &name) :
    StaticComplexAnimator(name) {
    mXAnimator = enve::make_shared<QrealAnimator>("x");
    mYAnimator = enve::make_shared<QrealAnimator>("y");
    ca_addChild(mXAnimator);
    ca_addChild(mYAnimator);
}

QPointF QPointFAnimator::getBaseValue() const {
    return QPointF(mXAnimator->getCurrentBaseValue(),
                   mYAnimator->getCurrentBaseValue());
}

QPointF QPointFAnimator::getEffectiveValue() const {
    return QPointF(mXAnimator->getEffectiveValue(),
                   mYAnimator->getEffectiveValue());
}

QPointF QPointFAnimator::getBaseValueAtAbsFrame(const qreal frame) const {
    return getBaseValue(prp_absFrameToRelFrameF(frame));
}

QPointF QPointFAnimator::getBaseValue(const qreal relFrame) const {
    return QPointF(mXAnimator->getBaseValue(relFrame),
                   mYAnimator->getBaseValue(relFrame));
}

QPointF QPointFAnimator::getEffectiveValueAtAbsFrame(const qreal frame) const {
    return getEffectiveValue(prp_absFrameToRelFrameF(frame));
}

QPointF QPointFAnimator::getEffectiveValue(const qreal relFrame) const {
    return QPointF(mXAnimator->getEffectiveValue(relFrame),
                   mYAnimator->getEffectiveValue(relFrame));
}

void QPointFAnimator::setPrefferedValueStep(const qreal valueStep) {
    mXAnimator->setPrefferedValueStep(valueStep);
    mYAnimator->setPrefferedValueStep(valueStep);
}

bool QPointFAnimator::getBeingTransformed() {
    return mXAnimator->getBeingTransformed() ||
            mYAnimator->getBeingTransformed();
}

qreal QPointFAnimator::getEffectiveXValue() {
    return mXAnimator->getEffectiveValue();
}

qreal QPointFAnimator::getEffectiveYValue() {
    return mYAnimator->getEffectiveValue();
}

qreal QPointFAnimator::getEffectiveXValue(const qreal relFrame) {
    return mXAnimator->getEffectiveValue(relFrame);
}

qreal QPointFAnimator::getEffectiveYValue(const qreal relFrame) {
    return mYAnimator->getEffectiveValue(relFrame);
}

void QPointFAnimator::setBaseValue(const QPointF &val) {
    mXAnimator->setCurrentBaseValue(val.x());
    mYAnimator->setCurrentBaseValue(val.y());
}

void QPointFAnimator::setValuesRange(const qreal minVal,
                                     const qreal maxVal) {
    mXAnimator->setValueRange(minVal, maxVal);
    mYAnimator->setValueRange(minVal, maxVal);
}

void QPointFAnimator::setBaseValueWithoutCallingUpdater(
        const QPointF &val) {
    mXAnimator->setCurrentBaseValueNoUpdate(val.x());
    mYAnimator->setCurrentBaseValueNoUpdate(val.y());
}

void QPointFAnimator::incBaseValuesWithoutCallingUpdater(
        const qreal x, const qreal y) {
    mXAnimator->incCurrentValueNoUpdate(x);
    mYAnimator->incCurrentValueNoUpdate(y);
}

void QPointFAnimator::incBaseValues(const qreal x, const qreal y) {
    mXAnimator->incCurrentBaseValue(x);
    mYAnimator->incCurrentBaseValue(y);
}

void QPointFAnimator::incAllBaseValues(const qreal x, const qreal y) {
    mXAnimator->incAllValues(x);
    mYAnimator->incAllValues(y);
}

void QPointFAnimator::incSavedValueToCurrentValue(const qreal incXBy,
                                                  const qreal incYBy) {
    mXAnimator->incSavedValueToCurrentValue(incXBy);
    mYAnimator->incSavedValueToCurrentValue(incYBy);
}

QrealAnimator *QPointFAnimator::getXAnimator() {
    return mXAnimator.data();
}

QrealAnimator *QPointFAnimator::getYAnimator() {
    return mYAnimator.data();
}

void QPointFAnimator::multSavedValueToCurrentValue(const qreal sx,
                                                   const qreal sy) {
    mXAnimator->multSavedValueToCurrentValue(sx);
    mYAnimator->multSavedValueToCurrentValue(sy);
}

void QPointFAnimator::multCurrentValues(const qreal sx, const qreal sy) {
    mXAnimator->multCurrentBaseValue(sx);
    mYAnimator->multCurrentBaseValue(sy);
}

QPointF QPointFAnimator::getSavedValue() {
    return QPointF(mXAnimator->getSavedBaseValue(),
                   mYAnimator->getSavedBaseValue());
}

qreal QPointFAnimator::getSavedXValue() {
    return mXAnimator->getSavedBaseValue();
}

qreal QPointFAnimator::getSavedYValue() {
    return mYAnimator->getSavedBaseValue();
}
