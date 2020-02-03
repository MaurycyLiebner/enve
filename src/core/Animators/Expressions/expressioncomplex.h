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

#ifndef EXPRESSIONCOMPLEX_H
#define EXPRESSIONCOMPLEX_H

#include "expressionsinglechild.h"
#include "expressionvariable.h"

using ExpressionVarSPtr = QSharedPointer<ExpressionVariable>;

class ExpressionComplex : public ExpressionSingleChild {
public:
    ExpressionComplex(const QList<ExpressionVarSPtr> &vars,
                      const sptr& value);

    sptr sCreate(const QList<ExpressionVarSPtr>& vars,
                 const ExpressionValue::sptr &value);

    qreal calculateValue(const qreal relFrame) const override;
    void collapse() override;
    QString toString() const override;
private:
    QList<ExpressionVarSPtr> mVariables;
};

#endif // EXPRESSIONCOMPLEX_H
