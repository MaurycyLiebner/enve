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

#ifndef BOOLPROPERTY_H
#define BOOLPROPERTY_H
#include "property.h"

class BoolProperty : public Property {
    e_OBJECT
protected:
    BoolProperty(const QString& name);
public:
    bool getValue();

    bool SWT_isBoolProperty() const { return true; }
    void writeProperty(eWriteStream& dst) const;
    void readProperty(eReadStream& src);
    void setValue(const bool value);
private:
    bool mValue = false;
};

#include "../Animators/staticcomplexanimator.h"
class BoolPropertyContainer : public StaticComplexAnimator {
    e_OBJECT
protected:
    BoolPropertyContainer(const QString& name);
public:
    bool SWT_isBoolPropertyContainer() const { return true; }
    void writeProperty(eWriteStream& dst) const;
    void readProperty(eReadStream& src);

    void setValue(const bool value);
    bool getValue();

    using ComplexAnimator::ca_addChild;
    using ComplexAnimator::ca_insertChild;
    using ComplexAnimator::ca_prependChildAnimator;
private:
    bool mValue = false;
};

#endif // BOOLPROPERTY_H
