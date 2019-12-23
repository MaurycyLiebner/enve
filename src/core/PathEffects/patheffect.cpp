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

#include "patheffect.h"
#include "pointhelpers.h"
#include "skia/skqtconversions.h"
#include "Properties/boolproperty.h"
#include "Animators/dynamiccomplexanimator.h"
#include "typemenu.h"

#include <QDrag>

PathEffect::PathEffect(const QString &name,
                       const PathEffectType type) :
    eEffect(name), mPathEffectType(type) {}

void PathEffect::writeIdentifier(eWriteStream& dst) const {
    dst.write(&mPathEffectType, sizeof(PathEffectType));
}

void PathEffect::prp_setupTreeViewMenu(PropertyMenu * const menu) {
    eEffect::prp_setupTreeViewMenu(menu);
    const PropertyMenu::PlainSelectedOp<PathEffect> dOp =
    [](PathEffect* const eff) {
        const auto parent = eff->getParent<DynamicComplexAnimatorBase<PathEffect>>();
        parent->removeChild(eff->ref<PathEffect>());
    };
    menu->addPlainAction("Delete Effect(s)", dOp);
}

PathEffectType PathEffect::getEffectType() {
    return mPathEffectType;
}

QMimeData *PathEffect::SWT_createMimeData() {
    return new eMimeData(QList<PathEffect*>() << this);
}

bool PathEffect::SWT_isPathEffect() const { return true; }
