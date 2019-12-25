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

#include "staticcomplexanimator.h"

StaticComplexAnimator::StaticComplexAnimator(const QString &name) :
    ComplexAnimator(name) {}

void StaticComplexAnimator::prp_writeProperty(eWriteStream &dst) const {
    const auto& children = ca_getChildren();
    for(const auto& prop : children)
        prop->prp_writeProperty(dst);
}

void StaticComplexAnimator::prp_readProperty(eReadStream &src) {
    const auto& children = ca_getChildren();
    for(const auto& prop : children)
        prop->prp_readProperty(src);
}
