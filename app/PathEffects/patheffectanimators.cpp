#include "patheffectanimators.h"
#include "PathEffects/patheffect.h"
#include "Boxes/pathbox.h"
#include "Boxes/boxesgroup.h"
#include <QDebug>

PathEffectAnimators::PathEffectAnimators(const bool &isOutline,
                                         const bool &isFill,
                                         BoundingBox *parentPath) :
    ComplexAnimator("path effects") {
    mIsOutline = isOutline;
    mIsFill = isFill;
    mParentBox = parentPath;
}

void PathEffectAnimators::addEffect(const qsptr<PathEffect>& effect) {
    if(mParentBox->SWT_isPathBox()) {
        const auto pathBox = GetAsPtr(mParentBox, PathBox);
        if(mIsOutline) {
            pathBox->addOutlinePathEffect(effect);
        } else if(mIsFill) {
            pathBox->addFillPathEffect(effect);
        } else {
            pathBox->addPathEffect(effect);
        }
    } else if(mParentBox->SWT_isBoxesGroup()) {
        const auto groupBox = GetAsPtr(mParentBox, BoxesGroup);
        if(mIsOutline) {
            groupBox->addOutlinePathEffect(effect);
        } else if(mIsFill) {
            groupBox->addFillPathEffect(effect);
        } else {
            groupBox->addPathEffect(effect);
        }
    }
}

void PathEffectAnimators::removeEffect(const qsptr<PathEffect>& effect) {
    if(mParentBox->SWT_isPathBox()) {
        const auto pathBox = GetAsPtr(mParentBox, PathBox);
        if(mIsOutline) {
            pathBox->removeOutlinePathEffect(effect);
        } else if(mIsFill) {
            pathBox->removeFillPathEffect(effect);
        } else {
            pathBox->removePathEffect(effect);
        }
    } else if(mParentBox->SWT_isBoxesGroup()) {
        const auto groupBox = GetAsPtr(mParentBox, BoxesGroup);
        if(mIsOutline) {
            groupBox->removeOutlinePathEffect(effect);
        } else if(mIsFill) {
            groupBox->removeFillPathEffect(effect);
        } else {
            groupBox->removePathEffect(effect);
        }
    }
}

BoundingBox *PathEffectAnimators::getParentBox() {
    return mParentBox;
}

const bool &PathEffectAnimators::isOutline() const {
    return mIsOutline;
}

const bool &PathEffectAnimators::isFill() const {
    return mIsFill;
}

bool PathEffectAnimators::hasEffects() {
    return !ca_mChildAnimators.isEmpty();
}

bool PathEffectAnimators::SWT_isPathEffectAnimators() const {
    return true;
}

void PathEffectAnimators::applyBeforeThickness(const qreal &relFrame,
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

void PathEffectAnimators::apply(const qreal &relFrame,
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
