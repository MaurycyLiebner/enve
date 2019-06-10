#include "patheffectanimators.h"
#include "PathEffects/patheffect.h"
#include "Boxes/pathbox.h"
#include "Boxes/containerbox.h"
#include <QDebug>

PathEffectAnimators::PathEffectAnimators(BoundingBox * const parentPath) :
    PathEffectAnimatorsBase("path effects") {
    mParentBox = parentPath;

    makeHiddenWhenEmpty();
}

void PathEffectAnimators::addEffect(const qsptr<PathEffect>& effect) {
    addChild(effect);
    const bool reasons = effect->hasReasonsNotToApplyUglyTransform();
    if(reasons && mParentBox->SWT_isPathBox()) {
        mParentBox->incReasonsNotToApplyUglyTransform();
    }
    SWT_setEnabled(true);
    SWT_setVisible(true);
}

void PathEffectAnimators::removeEffect(const qsptr<PathEffect>& effect) {
    removeChild(effect);
    const bool reasons = effect->hasReasonsNotToApplyUglyTransform();
    if(reasons && mParentBox->SWT_isPathBox()) {
        mParentBox->decReasonsNotToApplyUglyTransform();
    }
}

BoundingBox *PathEffectAnimators::getParentBox() {
    return mParentBox;
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
    addEffect(pathEffect);
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
    } else if(typeT == OPERATION_PATH_EFFECT) {
        pathEffect = SPtrCreate(OperationPathEffect)();
    } else if(typeT == LENGTH_PATH_EFFECT) {
        //pathEffect = SPtrCreate(LengthPathEffect)(mIsOutline);
        const auto subPathEffect = SPtrCreate(SubPathEffect)();
        subPathEffect->readLengthEffect(src);
        pathEffect = subPathEffect;
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
