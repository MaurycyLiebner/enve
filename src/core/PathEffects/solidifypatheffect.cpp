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

#include "solidifypatheffect.h"
#include "pathoperations.h"
#include "Boxes/pathbox.h"

SolidifyPathEffect::SolidifyPathEffect() :
    PathEffect("solidify effect", PathEffectType::SOLIDIFY) {
    mDisplacement = enve::make_shared<QrealAnimator>("displacement");
    mDisplacement->setValueRange(-999.999, 999.999);
    mDisplacement->setCurrentBaseValue(5);

    ca_addChild(mDisplacement);

    setPropertyForGUI(mDisplacement.get());
}

void SolidifyPathEffect::apply(const qreal relFrame,
                               const SkPath &src,
                               SkPath * const dst) {
    const qreal widthT = mDisplacement->getEffectiveValue(relFrame);
    gSolidify(widthT, src, dst);
}
