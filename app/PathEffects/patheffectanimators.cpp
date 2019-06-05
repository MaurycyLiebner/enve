#include "patheffectanimators.h"
#include "PathEffects/patheffect.h"
#include "Boxes/pathbox.h"
#include "Boxes/containerbox.h"
#include <QDebug>

PathEffectAnimators::PathEffectAnimators(const bool isOutline,
                                         const bool isFill,
                                         BoundingBox * const parentPath) :
    ComplexAnimator("path effects") {
    mIsOutline = isOutline;
    mIsFill = isFill;
    mParentBox = parentPath;

    SWT_setEnabled(false);
    SWT_setVisible(false);
}

void PathEffectAnimators::addEffect(const qsptr<PathEffect>& effect) {
    ca_addChildAnimator(effect);
    const bool reasons = effect->hasReasonsNotToApplyUglyTransform();
    if(reasons && mParentBox->SWT_isPathBox()) {
        mParentBox->incReasonsNotToApplyUglyTransform();
    }
    SWT_setEnabled(true);
    SWT_setVisible(true);
}

void PathEffectAnimators::removeEffect(const qsptr<PathEffect>& effect) {
    ca_removeChildAnimator(effect);
    const bool reasons = effect->hasReasonsNotToApplyUglyTransform();
    if(reasons && mParentBox->SWT_isPathBox()) {
        mParentBox->decReasonsNotToApplyUglyTransform();
    }
}

BoundingBox *PathEffectAnimators::getParentBox() {
    return mParentBox;
}

bool PathEffectAnimators::isOutline() const {
    return mIsOutline;
}

bool PathEffectAnimators::isFill() const {
    return mIsFill;
}

bool PathEffectAnimators::hasEffects() {
    return !ca_mChildAnimators.isEmpty();
}

bool PathEffectAnimators::SWT_isPathEffectAnimators() const {
    return true;
}

void PathEffectAnimators::applyBeforeThickness(const qreal relFrame,
                                               SkPath * const srcDstPath) {
    SkPath dstPath = *srcDstPath;
    for(const auto& effect : ca_mChildAnimators) {
        const auto& effectT = GetAsPtr(effect, PathEffect);
        if(effectT->applyBeforeThickness() && effectT->isVisible()) {
            SkPath srcPath = dstPath;
            effectT->apply(relFrame, srcPath, &dstPath);
        }
    }
    *srcDstPath = dstPath;
}

void PathEffectAnimators::apply(const qreal relFrame,
                                SkPath * const srcDstPath) {
    SkPath dstPath = *srcDstPath;
    for(const auto& effect : ca_mChildAnimators) {
        const auto& effectT = GetAsPtr(effect, PathEffect);
        if(effectT->applyBeforeThickness() || !effectT->isVisible()) {
            continue;
        }
        SkPath srcPath = dstPath;
        effectT->apply(relFrame, srcPath, &dstPath);
    }
    *srcDstPath = dstPath;
}
