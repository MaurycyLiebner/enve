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

#include "expressionoperator.h"
#include "expressionplainvalue.h"

ExpressionOperator::ExpressionOperator(
        const bool childrenNeedBrackets,
        const bool needsBrackets,
        const QString &symbol,
        const std::function<qreal(qreal, qreal)> &func,
        const sptr &value1, const sptr &value2) :
    ExpressionValue(needsBrackets),
    mChildrenNeedBrackets(childrenNeedBrackets),
    mSymbol(symbol), mFunc(func), mValue1(value1), mValue2(value2) {
    connect(mValue1.get(), &ExpressionValue::currentValueChanged,
            this, &ExpressionValue::updateValue);
    connect(mValue1.get(), &ExpressionValue::relRangeChanged,
            this, &ExpressionValue::relRangeChanged);
    connect(mValue2.get(), &ExpressionValue::currentValueChanged,
            this, &ExpressionValue::updateValue);
    connect(mValue2.get(), &ExpressionValue::relRangeChanged,
            this, &ExpressionValue::relRangeChanged);
}

ExpressionValue::sptr ExpressionOperator::sCreate(
        const bool childrenNeedBrackets,
        const bool needsBrackets,
        const QString& name,
        const std::function<qreal (qreal, qreal)> &func,
        const sptr &value1, const sptr &value2) {
    return sptr(new ExpressionOperator(childrenNeedBrackets,
                                       needsBrackets, name,
                                       func, value1, value2));
}

void ExpressionOperator::collapse() {
    mValue1->collapse();
    mValue2->collapse();
    if(mValue1->isPlainValue()) {
        mValue1 = ExpressionPlainValue::sCreate(mValue1->currentValue());
    }
    if(mValue2->isPlainValue()) {
        mValue2 = ExpressionPlainValue::sCreate(mValue2->currentValue());
    }
}

bool ExpressionOperator::isPlainValue() const {
    return mValue1->isPlainValue() &&
           mValue2->isPlainValue();
}

bool ExpressionOperator::isValid() const {
    return mValue1->isValid() &&
           mValue2->isValid();
}

FrameRange ExpressionOperator::identicalRange(const qreal relFrame) const {
    const auto id1 = mValue1->identicalRange(relFrame);
    const auto id2 = mValue2->identicalRange(relFrame);
    return id1*id2;
}

bool ExpressionOperator::setRelFrame(const qreal relFrame) {
    setRelFrameValue(relFrame);
    const bool changed1 = mValue1->setRelFrame(relFrame);
    const bool changed2 = mValue2->setRelFrame(relFrame);
    return changed1 || changed2;
}

QString ExpressionOperator::toString() const {
    QString str1;
    QString str2;
    const bool brackets1 = mChildrenNeedBrackets &&
                           mValue1->needsBrackets();
    if(brackets1) str1 = "(" + mValue1->toString() + ")";
    else str1 = mValue1->toString();

    const bool brackets2 = mChildrenNeedBrackets &&
                           mValue2->needsBrackets();
    if(brackets2) str2 = "(" + mValue2->toString() + ")";
    else str2 = mValue2->toString();
    return str1 + " " + mSymbol + " " + str2;
}
