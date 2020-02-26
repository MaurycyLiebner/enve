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

#include "expressionrandomfunction.h"

ExpressionValue::sptr ExpressionRandomFunction::sCreate(
        const ExpressionValue::sptr &value) {
    const auto result = new ExpressionRandomFunction("rand", value);
    result->updateValue();
    return sptr(result);
}

qreal ExpressionRandomFunction::calculateValue(const qreal relFrame) const {
    const qreal seed = innerValue(relFrame);
    if(isInteger4Dec(seed)) {
        QRandomGenerator rand(static_cast<quint32>(qRound(seed)));
        return rand.generateDouble();
    } else {
        QRandomGenerator fRand(static_cast<quint32>(qFloor(seed)));
        QRandomGenerator cRand(static_cast<quint32>(qCeil(seed)));
        const qreal cRandWeight = seed - qFloor(seed);
        return fRand.generateDouble()*(1 - cRandWeight) +
                cRand.generateDouble()*cRandWeight;
    }
}
