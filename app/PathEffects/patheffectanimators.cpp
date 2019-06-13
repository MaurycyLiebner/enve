#include "patheffectanimators.h"
#include "PathEffects/patheffect.h"
#include "Boxes/pathbox.h"
#include "Boxes/containerbox.h"
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
    PathEffectType typeT;
    src->read(rcChar(&typeT), sizeof(PathEffectType));
    qsptr<PathEffect> pathEffect;
    if(typeT == DISPLACE_PATH_EFFECT) {
        pathEffect = SPtrCreate(DisplacePathEffect)();
    } else if(typeT == DUPLICATE_PATH_EFFECT) {
        pathEffect = SPtrCreate(DuplicatePathEffect)();
    } else if(typeT == SOLIDIFY_PATH_EFFECT) {
        pathEffect = SPtrCreate(SolidifyPathEffect)();
    } else if(typeT == SUM_PATH_EFFECT) {
        pathEffect = SPtrCreate(SumPathEffect)();
    } else if(typeT == SUB_PATH_EFFECT) {
        pathEffect = SPtrCreate(SubPathEffect)();
    } else if(typeT == LINES_PATH_EFFECT) {
        pathEffect = SPtrCreate(LinesPathEffect)();
    } else if(typeT == ZIGZAG_PATH_EFFECT) {
        pathEffect = SPtrCreate(ZigZagPathEffect)();
    } else if(typeT == SPATIAL_DISPLACE_PATH_EFFECT) {
        pathEffect = SPtrCreate(SpatialDisplacePathEffect)();
    } else {
        RuntimeThrow("Invalid path effect type '" +
                     QString::number(typeT) + "'");
    }
    return pathEffect;
}
