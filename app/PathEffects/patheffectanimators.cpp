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
        const auto& effectT = GetAsPtr(effect, PathEffect);
        if(!effectT->isVisible()) continue;
        const SkPath srcPath = dstPath;
        effectT->apply(relFrame, srcPath, &dstPath);
    }
    *srcDstPath = dstPath;
}

void PathEffectAnimators::readPathEffect(QIODevice * const src) {
    const auto pathEffect = readIdCreatePathEffect(src);
    pathEffect->readProperty(src);
    addChild(pathEffect);
}

#include "patheffectsinclude.h"
qsptr<PathEffect> readIdCreatePathEffect(QIODevice * const src) {
    PathEffectType type;
    src->read(rcChar(&type), sizeof(PathEffectType));
    switch(type) {
        case(PathEffectType::DISPLACE):
            return SPtrCreate(DisplacePathEffect)();
        case(PathEffectType::DASH):
            return SPtrCreate(DashPathEffect)();
        case(PathEffectType::DUPLICATE):
            return SPtrCreate(DuplicatePathEffect)();
        case(PathEffectType::SOLIDIFY):
            return SPtrCreate(SolidifyPathEffect)();
        case(PathEffectType::SUM):
            return SPtrCreate(SumPathEffect)();
        case(PathEffectType::SUB):
            return SPtrCreate(SubPathEffect)();
        case(PathEffectType::LINES):
            return SPtrCreate(LinesPathEffect)();
        case(PathEffectType::ZIGZAG):
            return SPtrCreate(ZigZagPathEffect)();
        case(PathEffectType::SPATIAL_DISPLACE):
            return SPtrCreate(SpatialDisplacePathEffect)();
        case(PathEffectType::SUBDIVIDE):
            return SPtrCreate(SubdividePathEffect)();
        case(PathEffectType::CUSTOM): {
            const auto id = CustomIdentifier::sRead(src);
            return CustomPathEffectCreator::sCreateForIdentifier(id);
        }
        default: RuntimeThrow("Invalid path effect type '" +
                              QString::number(int(type)) + "'");
    }
}
