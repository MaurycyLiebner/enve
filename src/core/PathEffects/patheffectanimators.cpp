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

void PathEffectAnimators::apply(const qreal relFrame,
                                SkPath * const srcDstPath) {
    SkPath dstPath = *srcDstPath;
    for(const auto& effect : ca_mChildAnimators) {
        const auto pEffect = static_cast<PathEffect*>(effect.get());
        if(!pEffect->isVisible()) continue;
        const SkPath srcPath = dstPath;
        pEffect->apply(relFrame, srcPath, &dstPath);
    }
    *srcDstPath = dstPath;
}

void PathEffectAnimators::readPathEffect(eReadStream& src) {
    const auto pathEffect = readIdCreatePathEffect(src);
    pathEffect->readProperty(src);
    addChild(pathEffect);
}

#include "patheffectsinclude.h"
qsptr<PathEffect> readIdCreatePathEffect(eReadStream &src) {
    PathEffectType type;
    src.read(rcChar(&type), sizeof(PathEffectType));
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
