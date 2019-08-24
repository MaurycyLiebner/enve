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

#include "boolproperty.h"

BoolProperty::BoolProperty(const QString &name) :
    Property(name) {}

bool BoolProperty::getValue() {
    return mValue;
}

void BoolProperty::setValue(const bool value) {
    mValue = value;
    prp_afterWholeInfluenceRangeChanged();
}

void BoolProperty::writeProperty(eWriteStream& dst) const {
    dst << mValue;
}

void BoolProperty::readProperty(eReadStream& src) {
    src >> mValue;
}

BoolPropertyContainer::BoolPropertyContainer(const QString &name) :
    StaticComplexAnimator(name) {}

bool BoolPropertyContainer::getValue() {
    return mValue;
}

void BoolPropertyContainer::setValue(const bool value) {
    mValue = value;
    prp_afterWholeInfluenceRangeChanged();

    for(const auto& prop : ca_mChildAnimators) {
        //prop->SWT_setVisible(value);
        prop->SWT_setDisabled(!value);
    }
}


void BoolPropertyContainer::writeProperty(eWriteStream& dst) const {
    dst << mValue;
    StaticComplexAnimator::writeProperty(dst);
}

void BoolPropertyContainer::readProperty(eReadStream& src) {
    bool value;
    src >> value;
    setValue(value);
    StaticComplexAnimator::readProperty(src);
}
