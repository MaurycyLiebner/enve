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

#include "patheffectanimators.h"
#include "PathEffects/patheffect.h"
#include "Boxes/pathbox.h"
#include "Boxes/containerbox.h"
#include "custompatheffectcreator.h"
#include <QDebug>

PathEffectAnimators::PathEffectAnimators() :
    PathEffectAnimatorsBase("path effects") {
    makeHiddenWhenEmpty();
}

bool PathEffectAnimators::hasEffects() {
    return !ca_mChildAnimators.isEmpty();
}

bool PathEffectAnimators::SWT_isPathEffectAnimators() const {
    return true;
}

void PathEffectAnimators::readPathEffect(eReadStream& src) {
    const auto pathEffect = readIdCreatePathEffect(src);
    pathEffect->prp_readProperty(src);
    addChild(pathEffect);
}

#include "patheffectstask.h"
void PathEffectAnimators::addEffects(const qreal relFrame,
                                     QList<stdsptr<PathEffectCaller>>& list) const {
    for(const auto& effect : ca_mChildAnimators) {
        const auto pEffect = static_cast<PathEffect*>(effect.get());
        if(!pEffect->isVisible()) continue;
        list << pEffect->getEffectCaller(relFrame);
    }
}

#include "patheffectsinclude.h"
qsptr<PathEffect> readIdCreatePathEffect(eReadStream &src) {
    PathEffectType type;
    src.read(&type, sizeof(PathEffectType));
    switch(type) {
        case(PathEffectType::DISPLACE):
            return enve::make_shared<DisplacePathEffect>();
        case(PathEffectType::DASH):
            return enve::make_shared<DashPathEffect>();
        case(PathEffectType::DUPLICATE):
            return enve::make_shared<DuplicatePathEffect>();
        case(PathEffectType::SOLIDIFY):
            return enve::make_shared<SolidifyPathEffect>();
        case(PathEffectType::SUM):
            return enve::make_shared<SumPathEffect>();
        case(PathEffectType::SUB):
            return enve::make_shared<SubPathEffect>();
        case(PathEffectType::LINES):
            return enve::make_shared<LinesPathEffect>();
        case(PathEffectType::ZIGZAG):
            return enve::make_shared<ZigZagPathEffect>();
        case(PathEffectType::SPATIAL_DISPLACE):
            return enve::make_shared<SpatialDisplacePathEffect>();
        case(PathEffectType::SUBDIVIDE):
            return enve::make_shared<SubdividePathEffect>();
        case(PathEffectType::CUSTOM): {
            const auto id = CustomIdentifier::sRead(src);
            const auto eff = CustomPathEffectCreator::sCreateForIdentifier(id);
            if(eff) return eff;
            RuntimeThrow("Unrecogized CustomPathEffect identifier " + id.toString());
        } default: RuntimeThrow("Invalid path effect type '" +
                                QString::number(int(type)) + "'");
    }
}
