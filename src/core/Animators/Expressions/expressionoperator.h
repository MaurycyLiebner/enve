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

#ifndef EXPRESSIONOPERATOR_H
#define EXPRESSIONOPERATOR_H
#include "expressionvalue.h"

class ExpressionOperator : public ExpressionValue {
public:
    ExpressionOperator(const bool childrenNeedBrackets,
                       const bool needsBrackets, const QString& symbol,
                       const std::function<qreal(qreal, qreal)>& func,
                       const sptr &value1, const sptr &value2);

    static sptr sCreate(const bool childrenNeedBrackets,
                        const bool needsBrackets, const QString& name,
                        const std::function<qreal(qreal, qreal)>& func,
                        const sptr& value1, const sptr& value2);

    qreal calculateValue(const qreal relFrame) const override
    { return mFunc(value1(relFrame), value2(relFrame)); }
    void collapse() override;
    bool isPlainValue() const override;
    bool isValid() const override;
    FrameRange identicalRange(const qreal relFrame) const override;
    bool setRelFrame(const qreal relFrame) override;
    QString toString() const override;
private:
    qreal value1(const qreal relFrame) const
    { return mValue1->value(relFrame); }
    qreal value2(const qreal relFrame) const
    { return mValue2->value(relFrame); }

    const bool mChildrenNeedBrackets;
    const QString mSymbol;
    const std::function<qreal(qreal, qreal)> mFunc;
    sptr mValue1;
    sptr mValue2;
};

#endif // EXPRESSIONOPERATOR_H
