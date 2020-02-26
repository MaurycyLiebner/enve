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

#include "expressionmanualvariable.h"

ExpressionManualVariable::ExpressionManualVariable(const QString &name) :
    ExpressionValue(false), mName(name) {}

ExpressionManualVariable::sptr ExpressionManualVariable::sCreate(const QString &name) {
    return sptr(new ExpressionManualVariable(name));
}

qreal ExpressionManualVariable::calculateValue(const qreal relFrame) const {
    Q_UNUSED(relFrame)
    return mValue;
}

FrameRange ExpressionManualVariable::identicalRange(const qreal relFrame) const {
    Q_UNUSED(relFrame)
    return FrameRange::EMINMAX;
}

bool ExpressionManualVariable::setRelFrame(const qreal relFrame) {
    Q_UNUSED(relFrame)
    return false;
}

void ExpressionManualVariable::setValue(const qreal value) {
    mValue = value;
    updateValue();
}
