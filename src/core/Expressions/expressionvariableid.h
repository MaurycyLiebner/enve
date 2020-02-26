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

#ifndef EXPRESSIONVARIABLEID_H
#define EXPRESSIONVARIABLEID_H

class ExpressionManualVariableId {
    friend class ExpressionComplex;
    friend class ExpressionVariableId;
    ExpressionManualVariableId(const int id) : fId(id) {}
    int fId;
public:
    bool isValid() const { return fId >= 0; }
};

class ExpressionVariableId {
    friend class ExpressionComplex;
    ExpressionVariableId(const bool isManual, const int id) :
        fIsManual(isManual), fId(id) {}
    ExpressionVariableId(const ExpressionManualVariableId& id) :
        fIsManual(true), fId(id.fId) {}
    bool fIsManual;
    int fId;
public:
    ExpressionManualVariableId toManualVariableId() {
        if(!fIsManual) return ExpressionManualVariableId{-1};
        return ExpressionManualVariableId{fId};
    }
    bool isManual() const { return fIsManual; }
    bool isValid() const { return fId >= 0; }
};

#endif // EXPRESSIONVARIABLEID_H
