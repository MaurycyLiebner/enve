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

#include "qrealpoint.h"
#include "qrealkey.h"
#include "../Expressions/expression.h"
#include "../simpletask.h"
#include "typemenu.h"
#include "GUI/dialogsinterface.h"

QrealAnimator::QrealAnimator(const qreal iniVal,
                             const qreal minVal,
                             const qreal maxVal,
                             const qreal prefferdStep,
                             const QString &name) :
    GraphAnimator(name) {
    mCurrentBaseValue = iniVal;
    mClampMin = minVal;
    mClampMax = maxVal;
    mPrefferedValueStep = prefferdStep;
}

QrealAnimator::QrealAnimator(const QString &name) : GraphAnimator(name) {}

void QrealAnimator::prp_setupTreeViewMenu(PropertyMenu * const menu) {
    if(menu->hasActionsForType<QrealAnimator>()) return;
    menu->addedActionsForType<QrealAnimator>();

    const auto parentWidget = menu->getParentWidget();

    const PropertyMenu::PlainSelectedOp<QrealAnimator> sOp =
    [parentWidget](QrealAnimator * aTarget) {
        const auto& interface = DialogsInterface::instance();
        interface.showExpressionDialog(aTarget, parentWidget);
    };
    menu->addPlainAction("Set Expression", sOp);

    const PropertyMenu::PlainSelectedOp<QrealAnimator> cOp =
    [](QrealAnimator * aTarget) {
        aTarget->clearExpressionAction();
    };
    menu->addPlainAction("Clear Expression", cOp)->setEnabled(hasExpression());
    menu->addSeparator();
    Animator::prp_setupTreeViewMenu(menu);
}

void QrealAnimator::prp_writeProperty(eWriteStream& dst) const {
    anim_writeKeys(dst);
    dst << mCurrentBaseValue;
    dst << !!mExpression;
    if(mExpression) {
        dst << mExpression->bindingsString();
        dst << mExpression->definitionsString();
        dst << mExpression->scriptString();
    }
}

stdsptr<Key> QrealAnimator::anim_createKey() {
    return enve::make_shared<QrealKey>(this);
}

void QrealAnimator::prp_readProperty(eReadStream& src) {
    anim_readKeys(src);

    qreal val; src >> val;
    const auto evVersion = src.evFileVersion();
    if(evVersion > 8 && evVersion < 15) {
        QString expression; src >> expression;
    } else if(evVersion >= 15) {
        bool hasExpr; src >> hasExpr;
        if(hasExpr) {
            QString bindingsStr; src >> bindingsStr;
            QString definitionsStr; src >> definitionsStr;
            QString scriptStr; src >> scriptStr;
            SimpleTask::sScheduleContexted(this,
            [this, bindingsStr, definitionsStr, scriptStr]() {
                setExpression(Expression::sCreate(
                                  bindingsStr, definitionsStr, scriptStr, this,
                                  Expression::sQrealAnimatorTester));
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
        minMoveValue = mClampMin;
        maxMoveValue = mClampMax;
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
    mClampMin = minVal;
    mClampMax = maxVal;
    setCurrentBaseValue(mCurrentBaseValue);
}

void QrealAnimator::setMinValue(const qreal minVal) {
    mClampMin = minVal;
    setCurrentBaseValue(mCurrentBaseValue);
}

void QrealAnimator::setMaxValue(const qreal maxVal) {
    mClampMax = maxVal;
    setCurrentBaseValue(mCurrentBaseValue);
}

void QrealAnimator::incAllValues(const qreal valInc) {
    const auto& keys = anim_getKeys();
    for(const auto &key : keys) {
        static_cast<QrealKey*>(key)->incValue(valInc);
    }
    incCurrentBaseValue(valInc);
}

qreal QrealAnimator::getMinPossibleValue() {
    return mClampMin;
}

qreal QrealAnimator::getMaxPossibleValue() {
    return mClampMax;
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

bool QrealAnimator::prp_dependsOn(const Property* const prop) const {
    if(Property::prp_dependsOn(prop)) return true;
    if(!mExpression) return false;
    return mExpression->dependsOn(prop);
}

bool QrealAnimator::hasValidExpression() const {
    return mExpression ? mExpression->isValid() : false;
}

QString QrealAnimator::getExpressionBindingsString() const {
    if(!mExpression) return "";
    return mExpression->bindingsString();
}

QString QrealAnimator::getExpressionDefinitionsString() const {
    if(!mExpression) return "";
    return mExpression->definitionsString();
}

QString QrealAnimator::getExpressionScriptString() const {
    if(!mExpression) return "";
    return mExpression->scriptString();
}

void QrealAnimator::setExpressionAction(const qsptr<Expression> &expression) {
    {
        prp_pushUndoRedoName("Change Expression");
        UndoRedo ur;
        const auto oldValue = mExpression.sptr();
        const auto newValue = expression;
        ur.fUndo = [this, oldValue]() {
            setExpression(oldValue);
        };
        ur.fRedo = [this, newValue]() {
            setExpression(newValue);
        };
        prp_addUndoRedo(ur);
    }
    setExpression(expression);
}

void QrealAnimator::setExpression(const qsptr<Expression>& expression) {
    auto& conn = mExpression.assign(expression);
    if(expression) {
        const int absFrame = anim_getCurrentAbsFrame();
        expression->setAbsFrame(absFrame);
        conn << connect(expression.get(), &Expression::currentValueChanged,
                        this, [this]() {
            updateCurrentEffectiveValue();
        });
        conn << connect(expression.get(), &Expression::relRangeChanged,
                        this, [this](const FrameRange& range) {
            prp_afterChangedRelRange(range);
        });
    }
    updateCurrentEffectiveValue();
    prp_afterWholeInfluenceRangeChanged();
    emit expressionChanged();
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
                     mClampMin, mClampMax);
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
    if(isZero4Dec(relFrame - anim_getCurrentRelFrame()))
        return getEffectiveValue();
    if(mExpression) {
        const auto ret = mExpression->evaluate(relFrame);
        if(ret.isNumber()) return clamped(ret.toNumber());
    }
    return getBaseValue(relFrame);
}

qreal QrealAnimator::getCurrentBaseValue() const {
    return mCurrentBaseValue;
}

qreal QrealAnimator::getEffectiveValue() const {
    if(mExpression) return mCurrentEffectiveValue;
    else return mCurrentBaseValue;
}

bool QrealAnimator::assignCurrentBaseValue(const qreal newValue) {
    if(isZero4Dec(newValue - mCurrentBaseValue)) return false;
    mCurrentBaseValue = newValue;
    emit baseValueChanged(mCurrentBaseValue);
    if(mExpression) updateCurrentEffectiveValue();
    else emit effectiveValueChanged(mCurrentBaseValue);
    return true;
}

void QrealAnimator::setCurrentBaseValue(const qreal newValue) {
    if(assignCurrentBaseValue(clamped(newValue))) {
        const auto currKey = anim_getKeyOnCurrentFrame<QrealKey>();
        if(currKey) currKey->setValue(mCurrentBaseValue);
        else prp_afterWholeInfluenceRangeChanged();
    }
}

bool QrealAnimator::updateCurrentBaseValue() {
    const qreal newValue = calculateBaseValueAtRelFrame(anim_getCurrentRelFrame());
    return assignCurrentBaseValue(newValue);
}

bool QrealAnimator::updateCurrentEffectiveValue() {
    if(!mExpression) return false;
    const auto ret = mExpression->evaluate();
    if(!ret.isNumber()) return false;
    const qreal newValue = clamped(ret.toNumber());
    if(isZero4Dec(newValue - mCurrentEffectiveValue)) return false;
    mCurrentEffectiveValue = newValue;
    emit effectiveValueChanged(newValue);
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

bool QrealAnimator::updateExpressionRelFrame() {
    if(!mExpression) return false;
    const int absFrame = anim_getCurrentAbsFrame();
    return mExpression->setAbsFrame(absFrame);
}

void QrealAnimator::prp_afterFrameShiftChanged(const FrameRange &oldAbsRange,
                                               const FrameRange &newAbsRange) {
    GraphAnimator::prp_afterFrameShiftChanged(oldAbsRange, newAbsRange);
    updateExpressionRelFrame();
}

void QrealAnimator::anim_setAbsFrame(const int frame) {
    GraphAnimator::anim_setAbsFrame(frame);
    const bool baseValChanged = updateCurrentBaseValue();
    const bool exprValChanged = updateExpressionRelFrame();
    const bool changed = baseValChanged || exprValChanged;
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
    startBaseValueTransform();

    const qreal currentValue = mCurrentBaseValue;
    GraphAnimator::anim_removeAllKeys();

    setCurrentBaseValue(currentValue);
    finishBaseValueTransform();
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
        return {mClampMin, mClampMax};
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
    assignCurrentBaseValue(clamped(newValue));
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
    startBaseValueTransform();
}

#include "canvas.h"
void QrealAnimator::prp_finishTransform() {
    if(const auto key = anim_getKeyOnCurrentFrame<QrealKey>()) {
        if(!mTransformed) return;
        mTransformed = false;
        key->finishValueTransform();
    } else {
        finishBaseValueTransform();
    }
}

void QrealAnimator::startBaseValueTransform() {
    if(mTransformed) return;
    mSavedCurrentValue = mCurrentBaseValue;
    mTransformed = true;
}

void QrealAnimator::finishBaseValueTransform() {
    if(!mTransformed) return;
    mTransformed = false;
    {
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
    if(!mTransformed) return;
    mTransformed = false;

    if(const auto key = anim_getKeyOnCurrentFrame<QrealKey>()) {
        key->cancelValueTransform();
    } else {
        setCurrentBaseValue(mSavedCurrentValue);
    }
}

FrameRange QrealAnimator::prp_getIdenticalRelRange(const int relFrame) const {
    const auto base = Animator::prp_getIdenticalRelRange(relFrame);
    if(mExpression) return base * mExpression->identicalRange(relFrame);
    else return base;
}

void QrealAnimator::prp_afterChangedAbsRange(const FrameRange &range,
                                             const bool clip) {
    if(range.inRange(anim_getCurrentAbsFrame()))
        updateCurrentBaseValue();
    GraphAnimator::prp_afterChangedAbsRange(range, clip);
}

void QrealAnimator::multCurrentBaseValue(const qreal mult) {
    setCurrentBaseValue(mCurrentBaseValue*mult);
}

qreal QrealAnimator::getSavedBaseValue() {
    return mSavedCurrentValue;
}

void QrealAnimator::saveQrealSVG(QDomDocument& doc,
                                 QDomElement& parent,
                                 QDomElement& defs,
                                 const FrameRange& absRange,
                                 const qreal fps,
                                 const QString& attrName,
                                 const qreal multiplier,
                                 const bool transform,
                                 const QString& type) const {
    Animator::saveSVG(doc, parent, defs, absRange, fps, attrName,
                      [this, multiplier](const int relFrame) {
        return QString::number(getEffectiveValue(relFrame)*multiplier);
    }, transform, type);
}
