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

#include "qrealanimator.h"
#include <QMenu>
#include "qrealpoint.h"
#include "qrealkey.h"
#include "randomqrealgenerator.h"
#include "Expressions/expressionvalue.h"
#include "Expressions/expressionparser.h"
#include "simpletask.h"

QrealAnimator::QrealAnimator(const qreal iniVal,
                             const qreal minVal,
                             const qreal maxVal,
                             const qreal prefferdStep,
                             const QString &name) :
    GraphAnimator(name) {
    mCurrentBaseValue = iniVal;
    mMinPossibleVal = minVal;
    mMaxPossibleVal = maxVal;
    mPrefferedValueStep = prefferdStep;
}

QrealAnimator::QrealAnimator(const QString &name) : GraphAnimator(name) {}

void QrealAnimator::prp_writeProperty(eWriteStream& dst) const {
    anim_writeKeys(dst);
    dst << mCurrentBaseValue;
    dst << (mExpression ? mExpression->toString() : "");
}

stdsptr<Key> QrealAnimator::anim_createKey() {
    return enve::make_shared<QrealKey>(this);
}

void QrealAnimator::prp_readProperty(eReadStream& src) {
    anim_readKeys(src);

    qreal val; src >> val;
    if(src.evFileVersion() > 8) {
        QString expression; src >> expression;
        if(!expression.isEmpty()) {
            SimpleTask::sSchedule([this, expression]() {
                const auto exp = ExpressionParser::parse(expression, this);
                setExpression(exp);
            });
        }
    }
    if(!anim_hasKeys()) setCurrentBaseValue(val);
}

void QrealAnimator::graph_getValueConstraints(
        GraphKey *key, const QrealPointType type,
        qreal &minMoveValue, qreal &maxMoveValue) const {
    Q_UNUSED(key)
    if(type == QrealPointType::c1Pt) {
        minMoveValue = -DBL_MAX;
        maxMoveValue = DBL_MAX;
//        auto nextKey = key->getNextKey<GraphKey>();
//        if(!nextKey) {
//            minMoveValue = mMinPossibleVal;
//            maxMoveValue = mMaxPossibleVal;
//            return;
//        }
//        qreal p0 = key->getValueForGraph();
//        qreal p2 = nextKey->getC0Value();
//        qreal p3 = nextKey->getValueForGraph();
//        int iMax = 2*(nextKey->getRelFrame() - key->getRelFrame());
//        minMoveValue = DBL_MIN;
//        maxMoveValue = DBL_MAX;
//        for(int i = 1; i < iMax; i++) {
//            qreal t = static_cast<qreal>(i)/iMax;
//            qreal maxVal = gSolveForP1(p0, p2, p3, t, mMaxPossibleVal);
//            maxMoveValue = qMin(maxMoveValue, maxVal);
//            qreal minVal = gSolveForP1(p0, p2, p3, t, mMinPossibleVal);
//            minMoveValue = qMax(minMoveValue, minVal);
//        }
    } else if(type == QrealPointType::c0Pt) {
        minMoveValue = -DBL_MAX;
        maxMoveValue = DBL_MAX;
//        auto prevKey = key->getPrevKey<GraphKey>();
//        if(!prevKey) {
//            minMoveValue = mMinPossibleVal;
//            maxMoveValue = mMaxPossibleVal;
//            return;
//        }
//        qreal p0 = prevKey->getValueForGraph();
//        qreal p1 = prevKey->getC1Value();
//        qreal p3 = key->getValueForGraph();
//        int iMax = 2*(key->getRelFrame() - prevKey->getRelFrame());
//        minMoveValue = DBL_MIN;
//        maxMoveValue = DBL_MAX;
//        for(int i = 1; i < iMax; i++) {
//            qreal t = static_cast<qreal>(i)/iMax;
//            qreal maxVal = gSolveForP2(p0, p1, p3, t, mMaxPossibleVal);
//            maxMoveValue = qMin(maxMoveValue, maxVal);
//            qreal minVal = gSolveForP2(p0, p1, p3, t, mMinPossibleVal);
//            minMoveValue = qMax(minMoveValue, minVal);
//        }
    } else { // keyPt
        minMoveValue = mMinPossibleVal;
        maxMoveValue = mMaxPossibleVal;
    }
}

QrealSnapshot QrealAnimator::makeSnapshot(
        const qreal frameMultiplier,
        const qreal valueMultiplier) const {
    QrealSnapshot snapshot(mCurrentBaseValue, frameMultiplier, valueMultiplier);
    const auto& keys = anim_getKeys();
    for(const auto& key : keys) {
        const auto qaKey = static_cast<QrealKey*>(key);
        snapshot.appendKey(qaKey);
    }
    return snapshot;
}

QrealSnapshot QrealAnimator::makeSnapshot(
        const int minFrame, const int maxFrame,
        const qreal frameMultiplier,
        const qreal valueMultiplier) const {
    const int prevKeyId = anim_getPrevKeyId(minFrame);
    const int nextKeyId = anim_getNextKeyId(maxFrame);
    const auto& keys = anim_getKeys();
    const int minI = clamp(prevKeyId, 0, keys.count() - 1);
    const int maxI = clamp(nextKeyId, 0, keys.count() - 1);
    QrealSnapshot snapshot(mCurrentBaseValue, frameMultiplier, valueMultiplier);
    for(int i = minI; i <= maxI; i++) {
        const auto iKey = keys.atId<QrealKey>(i);
        snapshot.appendKey(iKey);
    }
    return snapshot;
}

void QrealAnimator::setValueRange(const qreal minVal, const qreal maxVal) {
    mMinPossibleVal = minVal;
    mMaxPossibleVal = maxVal;
    setCurrentBaseValue(mCurrentBaseValue);
}

void QrealAnimator::incAllValues(const qreal valInc) {
    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        static_cast<QrealKey*>(key)->incValue(valInc);
    }
    incCurrentBaseValue(valInc);
}

QString QrealAnimator::prp_getValueText() {
    return QString::number(mCurrentBaseValue, 'f', 2);
}

qreal QrealAnimator::getMinPossibleValue() {
    return mMinPossibleVal;
}

qreal QrealAnimator::getMaxPossibleValue() {
    return mMaxPossibleVal;
}

qreal QrealAnimator::getPrefferedValueStep() {
    return mPrefferedValueStep;
}

void QrealAnimator::setPrefferedValueStep(const qreal valueStep) {
    mPrefferedValueStep = valueStep;
}

void QrealAnimator::graphFixMinMaxValues() {
    mGraphMinMaxValuesFixed = true;
}

qreal QrealAnimator::getEffectiveValueAtAbsFrame(const qreal frame) const {
    return getEffectiveValue(prp_absFrameToRelFrameF(frame));
}

qreal QrealAnimator::getBaseValueAtAbsFrame(const qreal frame) const {
    return getBaseValue(prp_absFrameToRelFrameF(frame));
}

void QrealAnimator::setGenerator(const qsptr<RandomQrealGenerator>& generator) {
    if(generator == mRandomGenerator.data()) return;

    if(!generator) {
        mRandomGenerator.reset();
    } else {
        mRandomGenerator = generator->ref<RandomQrealGenerator>();
    }

    prp_afterWholeInfluenceRangeChanged();
}

bool QrealAnimator::hasNoise() {
    return !mRandomGenerator.isNull();
}

bool QrealAnimator::hasValidExpression() const {
    return mExpression ? mExpression->isValid() : false;
}

QString QrealAnimator::expressionText() const {
    return mExpression ? mExpression->toString() : "";
}

void QrealAnimator::setExpression(const qsptr<ExpressionValue> &expression) {
    auto& conn = mExpression.assign(expression);
    if(expression) {
        expression->setRelFrame(anim_getCurrentRelFrame());
        conn << connect(expression.get(), &ExpressionValue::currentValueChanged,
                        this, &QrealAnimator::effectiveValueChanged);
        conn << connect(expression.get(), &ExpressionValue::relRangeChanged,
                        this, [this](const FrameRange& range) {
            prp_afterChangedRelRange(range);
        });
    }
    emit effectiveValueChanged(getEffectiveValue());
    prp_afterWholeInfluenceRangeChanged();
}

qreal QrealAnimator::calculateBaseValueAtRelFrame(const qreal frame) const {
    if(!anim_hasKeys()) return mCurrentBaseValue;
    const auto pn = anim_getPrevAndNextKeyIdF(frame);
    const int prevId = pn.first;
    const int nextId = pn.second;

    const bool adjKeys = nextId - prevId == 1;
    const auto keyAtRelFrame = adjKeys ? nullptr :
                               anim_getKeyAtIndex<QrealKey>(prevId + 1);
    if(keyAtRelFrame) return keyAtRelFrame->getValue();
    const auto prevKey = anim_getKeyAtIndex<QrealKey>(prevId);
    const auto nextKey = anim_getKeyAtIndex<QrealKey>(nextId);

    if(prevKey && nextKey) {
        const qCubicSegment1D seg{qreal(prevKey->getRelFrame()),
                                  prevKey->getC1Frame(),
                                  nextKey->getC0Frame(),
                                  qreal(nextKey->getRelFrame())};
        const qreal t = gTFromX(seg, frame);
        const qreal p0y = prevKey->getValue();
        const qreal p1y = prevKey->getC1Value();
        const qreal p2y = nextKey->getC0Value();
        const qreal p3y = nextKey->getValue();
        return clamp(gCubicValueAtT({p0y, p1y, p2y, p3y}, t),
                     mMinPossibleVal, mMaxPossibleVal);
    } else if(prevKey) {
        return prevKey->getValue();
    } else if(nextKey) {
        return nextKey->getValue();
    }
    return mCurrentBaseValue;
}

qreal QrealAnimator::getBaseValue(const qreal relFrame) const {
    if(isZero4Dec(relFrame - anim_getCurrentRelFrame()))
        return mCurrentBaseValue;
    return calculateBaseValueAtRelFrame(relFrame);
}

qreal QrealAnimator::getEffectiveValue(const qreal relFrame) const {
    if(mExpression) return mExpression->value(relFrame);
    if(mRandomGenerator.isNull()) return getBaseValue(relFrame);
    const qreal val = getBaseValue(relFrame) +
            mRandomGenerator->getDevAtRelFrame(relFrame);
    return qMin(mMaxPossibleVal, qMax(mMinPossibleVal, val));
}

qreal QrealAnimator::getCurrentBaseValue() const {
    return mCurrentBaseValue;
}

qreal QrealAnimator::getEffectiveValue() const {
    if(mExpression) return qBound(mMinPossibleVal,
                                  mExpression->currentValue(),
                                  mMaxPossibleVal);
    if(mRandomGenerator.isNull()) return mCurrentBaseValue;
    const qreal val = mCurrentBaseValue +
            mRandomGenerator->getDevAtRelFrame(anim_getCurrentRelFrame());
    return qMin(mMaxPossibleVal, qMax(mMinPossibleVal, val));
}

void QrealAnimator::setCurrentBaseValue(qreal newValue) {
    newValue = clamp(newValue, mMinPossibleVal, mMaxPossibleVal);

    if(isZero4Dec(newValue - mCurrentBaseValue)) return;

    mCurrentBaseValue = newValue;
    const auto currKey = anim_getKeyOnCurrentFrame<QrealKey>();
    if(currKey) currKey->setValue(mCurrentBaseValue);
    else prp_afterWholeInfluenceRangeChanged();

    emit baseValueChanged(mCurrentBaseValue);
    emit effectiveValueChanged(getEffectiveValue());

    //anim_updateKeysPath();
}

bool QrealAnimator::updateBaseValueFromCurrentFrame() {
    const qreal newValue = calculateBaseValueAtRelFrame(anim_getCurrentRelFrame());
    if(isZero4Dec(newValue - mCurrentBaseValue)) return false;
    mCurrentBaseValue = newValue;
    emit baseValueChanged(mCurrentBaseValue);
    emit effectiveValueChanged(getEffectiveValue());
    return true;
}

void QrealAnimator::saveValueToKey(const int frame, const qreal value) {
    const auto keyAtFrame = anim_getKeyAtAbsFrame<QrealKey>(frame);
    if(keyAtFrame) {
        keyAtFrame->setValue(value);
    } else {
        const auto newKey = enve::make_shared<QrealKey>(value, frame, this);
        anim_appendKey(newKey);
    }
}

void QrealAnimator::anim_setAbsFrame(const int frame) {
    Animator::anim_setAbsFrame(frame);
    bool changed = updateBaseValueFromCurrentFrame();
    if(mExpression) {
        const bool exprValChanged = mExpression->setRelFrame(frame);
        changed = changed || exprValChanged;
    }
    if(changed) prp_afterChangedCurrent(UpdateReason::frameChange);
}

void QrealAnimator::anim_addKeyAtRelFrame(const int relFrame) {
    if(anim_getKeyAtRelFrame(relFrame)) return;
    const qreal value = getBaseValue(relFrame);
    const auto newKey = enve::make_shared<QrealKey>(value, relFrame, this);
    anim_appendKeyAction(newKey);
}

void QrealAnimator::anim_removeAllKeys() {
    if(!anim_hasKeys()) return;
    const qreal currentValue = mCurrentBaseValue;

    const auto keys = anim_getKeys();
    for(const auto& key : keys) {
        anim_removeKey(key->ref<Key>());
    }
    setCurrentBaseValue(currentValue);
}

QPainterPath QrealAnimator::graph_getPathForSegment(
        const GraphKey * const prevKey,
        const GraphKey * const nextKey) const {
    QPainterPath path;
    if(prevKey) {
        path.moveTo(prevKey->getRelFrame(), prevKey->getValueForGraph());
        if(nextKey) {
            path.cubicTo(QPointF(prevKey->getC1Frame(),
                                 prevKey->getC1Value()),
                         QPointF(nextKey->getC0Frame(),
                                 nextKey->getC0Value()),
                         QPointF(nextKey->getRelFrame(),
                                 nextKey->getValueForGraph()));
        } else {
            path.lineTo(50000, prevKey->getValueForGraph());
        }
    } else {
        if(nextKey) {
            const qreal nextVal = nextKey->getValueForGraph();
            path.moveTo(-50000, nextVal);
            path.lineTo(nextKey->getRelFrame(), nextVal);
        } else {
            path.moveTo(-50000, mCurrentBaseValue);
            path.lineTo(50000, mCurrentBaseValue);
        }
    }
    return path;
}

qValueRange QrealAnimator::graph_getMinAndMaxValues() const {
    if(mGraphMinMaxValuesFixed) {
        return {mMinPossibleVal, mMaxPossibleVal};
    }
    if(!anim_hasKeys()) {
        return {mCurrentBaseValue - mPrefferedValueStep,
                mCurrentBaseValue + mPrefferedValueStep};
    }
    qreal minVal = TEN_MIL;
    qreal maxVal = -TEN_MIL;
    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto qaKey = static_cast<QrealKey*>(key);
        const qreal keyVal = qaKey->getValue();
        const qreal startVal = qaKey->getC0Value();
        const qreal endVal = qaKey->getC1Value();
        const qreal maxKeyVal = qMax(qMax(startVal, endVal), keyVal);
        const qreal minKeyVal = qMin(qMin(startVal, endVal), keyVal);
        if(maxKeyVal > maxVal) maxVal = maxKeyVal;
        if(minKeyVal < minVal) minVal = minKeyVal;
    }

    return {minVal - mPrefferedValueStep, maxVal + mPrefferedValueStep};
}

qValueRange QrealAnimator::graph_getMinAndMaxValuesBetweenFrames(
        const int startFrame, const int endFrame) const {
    if(!anim_hasKeys()) return {mCurrentBaseValue, mCurrentBaseValue};
    bool first = true;
    qreal minVal = TEN_MIL;
    qreal maxVal = -TEN_MIL;
    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        const auto qaKey = static_cast<QrealKey*>(key);
        const int keyFrame = key->getAbsFrame();
        if(keyFrame > endFrame || keyFrame < startFrame) continue;
        if(first) first = false;
        const qreal keyVal = qaKey->getValue();
        const qreal startVal = qaKey->getC0Value();
        const qreal endVal = qaKey->getC1Value();
        const qreal maxKeyVal = qMax(qMax(startVal, endVal), keyVal);
        const qreal minKeyVal = qMin(qMin(startVal, endVal), keyVal);
        if(maxKeyVal > maxVal) maxVal = maxKeyVal;
        if(minKeyVal < minVal) minVal = minKeyVal;
    }
    if(first) {
        const int midFrame = (startFrame + endFrame)/2;
        const qreal value = getBaseValueAtAbsFrame(midFrame);
        return {value, value};
    }
    return {minVal, maxVal};
}

qreal QrealAnimator::graph_clampGraphValue(const qreal value) {
    return value;
}

void QrealAnimator::incSavedValueToCurrentValue(const qreal incBy) {
    setCurrentBaseValue(mSavedCurrentValue + incBy);
}

void QrealAnimator::multSavedValueToCurrentValue(const qreal multBy) {
    setCurrentBaseValue(mSavedCurrentValue * multBy);
}

void QrealAnimator::setCurrentBaseValueNoUpdate(const qreal newValue) {
    mCurrentBaseValue = clamp(newValue, mMinPossibleVal, mMaxPossibleVal);
    emit baseValueChanged(mCurrentBaseValue);
    emit effectiveValueChanged(getEffectiveValue());
}

void QrealAnimator::incCurrentValueNoUpdate(const qreal incBy) {
    setCurrentBaseValueNoUpdate(mCurrentBaseValue + incBy);
}

void QrealAnimator::incCurrentBaseValue(const qreal incBy) {
    setCurrentBaseValue(mCurrentBaseValue + incBy);
}

void QrealAnimator::prp_startTransform() {
    if(mTransformed) return;
    if(anim_isRecording() && !anim_getKeyOnCurrentFrame()) {
        anim_saveCurrentValueAsKey();
    }
    if(const auto key = anim_getKeyOnCurrentFrame<QrealKey>()) {
        key->startValueTransform();
    }
    mSavedCurrentValue = mCurrentBaseValue;
    mTransformed = true;
}

#include "canvas.h"
void QrealAnimator::prp_finishTransform() {
    if(!mTransformed) return;
    mTransformed = false;

    if(const auto key = anim_getKeyOnCurrentFrame<QrealKey>()) {
        key->finishValueTransform();
    } else {
        UndoRedo ur;
        const qreal oldValue = mSavedCurrentValue;
        const qreal newValue = mCurrentBaseValue;
        ur.fUndo = [this, oldValue]() {
            setCurrentBaseValue(oldValue);
        };
        ur.fRedo = [this, newValue]() {
            setCurrentBaseValue(newValue);
        };
        prp_addUndoRedo(ur);
    }
}

void QrealAnimator::prp_cancelTransform() {
    if(mTransformed) {
        mTransformed = false;
        setCurrentBaseValue(mSavedCurrentValue);
    }
}

FrameRange QrealAnimator::prp_getIdenticalRelRange(const int relFrame) const {
    const auto base = Animator::prp_getIdenticalRelRange(relFrame);
    if(mExpression) return base * mExpression->identicalRange(relFrame);
    else return base;
}

void QrealAnimator::multCurrentBaseValue(const qreal mult) {
    setCurrentBaseValue(mCurrentBaseValue*mult);
}

qreal QrealAnimator::getSavedBaseValue() {
    return mSavedCurrentValue;
}
