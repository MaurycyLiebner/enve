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

#include "expressioncomplex.h"

ExpressionComplex::ExpressionComplex(
        const QList<PlainVarSPtr>& plainVars,
        const QList<ExpressionVarSPtr> &vars,
        const sptr &value) :
    ExpressionSingleChild("Complex", value),
    mManualVariables(plainVars),
    mVariables(vars) {}

ExpressionValue::sptr ExpressionComplex::sCreate(
        const QList<PlainVarSPtr>& plainVars,
        const QList<ExpressionVarSPtr>& vars,
        const ExpressionValue::sptr& value) {
    const auto result = new ExpressionComplex(plainVars, vars, value);
    result->updateValue();
    return sptr(result);
}

qreal ExpressionComplex::calculateValue(const qreal relFrame) const {
    return innerValue(relFrame);
}

void ExpressionComplex::collapse() {
    for(const auto& var : mVariables)
        var->collapse();
    ExpressionSingleChild::collapse();
}

QString ExpressionComplex::toString() const {
    QString result;
    for(const auto& var : mVariables)
        result += var->definitionString();
    result += innerString();
    return result;
}

ExpressionManualVariableId ExpressionComplex::getManualVariableId(
        const QString &name) const {
    const int iMax = mManualVariables.count();
    for(int i = 0; i < iMax; i++) {
        const auto& var = mManualVariables.at(i);
        if(var->name() == name) return ExpressionManualVariableId{i};
    }
    return ExpressionManualVariableId{-1};
}

void ExpressionComplex::setManualVariableValue(
        const ExpressionManualVariableId &variableId, const qreal value) {
    const int id = variableId.fId;
    if(id < 0 || id > mManualVariables.count()) return;
    mManualVariables.at(id)->setValue(value);
}

ExpressionVariableId ExpressionComplex::getVariableId(const QString &name) const {
    const auto manualVar = getManualVariableId(name);
    if(manualVar.isValid()) return manualVar;
    const int iMax = mVariables.count();
    for(int i = 0; i < iMax; i++) {
        const auto& var = mVariables.at(i);
        if(var->name() == name) return ExpressionVariableId{false, i};
    }
    return ExpressionVariableId{false, -1};
}

qreal ExpressionComplex::getVariableValue(const ExpressionVariableId &id) const {
    if(id.isManual()) {
        return mManualVariables.at(id.fId)->currentValue();
    } else {
        return mVariables.at(id.fId)->currentValue();
    }
}
