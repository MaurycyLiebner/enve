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

#ifndef INTPROPERTY_H
#define INTPROPERTY_H
#include "property.h"

class IntProperty : public Property {
    Q_OBJECT
    e_OBJECT
protected:
    IntProperty(const QString& name);
public:
    bool SWT_isIntProperty() const;

    void prp_writeProperty(eWriteStream& dst) const;
    void prp_readProperty(eReadStream& src);
    void prp_startTransform();
    void prp_finishTransform();

    void setValueRange(const int minValue, const int maxValue);

    void setCurrentValue(const int value);
    int getValue();

    int getMaxValue();
    int getMinValue();
protected:
    bool mTransformed = false;
    int mValue = 0;
    int mMinValue = 0;
    int mMaxValue = 9999;
signals:
    void valueChangedSignal(int);
};

#endif // INTPROPERTY_H
