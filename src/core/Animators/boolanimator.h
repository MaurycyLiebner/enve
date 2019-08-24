// enve - 2D animations software
// Copyright (C) 2016-2019 Maurycy Liebner

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

#ifndef BOOLANIMATOR_H
#define BOOLANIMATOR_H
#include "intanimator.h"

class BoolAnimator : public IntAnimator {
    e_OBJECT
public:
    bool getCurrentBoolValue();
    void setCurrentBoolValue(const bool bT);
    bool getCurrentBoolValueAtRelFrame(const qreal relFrame) {
        return getBaseIntValue(relFrame) == 1;
    }

    bool SWT_isBoolAnimator() const { return true; }
protected:
    BoolAnimator(const QString& name);
};

#endif // BOOLANIMATOR_H
