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

#ifndef EXPRESSIONPLAINVALUE_H
#define EXPRESSIONPLAINVALUE_H

#include "expressionvalue.h"

class ExpressionPlainValue : public ExpressionValue {
public:
    ExpressionPlainValue(const qreal value);
    ExpressionPlainValue(const QString& string, const qreal value);

    using sptr = QSharedPointer<ExpressionPlainValue>;
    static sptr sCreate(const QString &string, const qreal value);
    static sptr sCreate(const qreal value);

    qreal calculateValue(const qreal relFrame) const override
    { Q_UNUSED(relFrame) return mValue; }
    bool isPlainValue() const override { return true; }
    bool isValid() const override { return true; }
    void collapse() override {}
    FrameRange identicalRange(const qreal relFrame) const override
    { Q_UNUSED(relFrame) return FrameRange::EMINMAX; }
    QString toString() const override
    { return mString; }
private:
    const QString mString;
    const qreal mValue;
};

#endif // EXPRESSIONPLAINVALUE_H
