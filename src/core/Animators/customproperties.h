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

#ifndef CUSTOMPROPERTIES_H
#define CUSTOMPROPERTIES_H
#include "dynamiccomplexanimator.h"

qsptr<Animator> readIdCreateCProperty(eReadStream& src);
void writeCPropertyType(Animator* const obj, eWriteStream& dst);

using CustomPropertiesBase =
    DynamicComplexAnimator<Animator,
                           writeCPropertyType,
                           readIdCreateCProperty>;

class CustomProperties : public CustomPropertiesBase {
    Q_OBJECT
    e_OBJECT
protected:
    CustomProperties();
public:
    void prp_setupTreeViewMenu(PropertyMenu * const menu) override;

    void prp_writeProperty(eWriteStream& dst) const override;
    void prp_readProperty(eReadStream& src) override;

    void addProperty(const qsptr<Animator>& prop);
private:
    using CustomPropertiesBase::addChild;
    using CustomPropertiesBase::insertChild;
};

#endif // CUSTOMPROPERTIES_H
