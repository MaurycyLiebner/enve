#include "patheffectanimators.h"
#include "patheffect.h"
#include "Boxes/pathbox.h"
#include "Boxes/boxesgroup.h"
#include <QDebug>

PathEffectAnimators::PathEffectAnimators(const bool &isOutline,
                                         const bool &isFill,
                                         BoundingBox *parentPath) :
    ComplexAnimator() {
    mIsOutline = isOutline;
    mIsFill = isFill;
    mParentBox = parentPath;
}

void PathEffectAnimators::addEffect(PathEffect *effect) {
    if(mParentBox->SWT_isPathBox()) {
        PathBox *pathBox = (PathBox*)mParentBox;
        if(mIsOutline) {
            pathBox->addOutlinePathEffect(effect);
        } else if(mIsFill) {
            pathBox->addFillPathEffect(effect);
        } else {
            pathBox->addPathEffect(effect);
        }
    } else if(mParentBox->SWT_isPathBox()) {
        BoxesGroup *groupBox = (BoxesGroup*)mParentBox;
        if(mIsOutline) {
            groupBox->addOutlinePathEffect(effect);
        } else if(mIsFill) {
            groupBox->addFillPathEffect(effect);
        } else {
            groupBox->addPathEffect(effect);
        }
    }
}

void PathEffectAnimators::removeEffect(PathEffect *effect) {
    if(mParentBox->SWT_isPathBox()) {
        PathBox *pathBox = (PathBox*)mParentBox;
        if(mIsOutline) {
            pathBox->removeOutlinePathEffect(effect);
        } else if(mIsFill) {
            pathBox->removeFillPathEffect(effect);
        } else {
            pathBox->removePathEffect(effect);
        }
    } else if(mParentBox->SWT_isPathBox()) {
        BoxesGroup *groupBox = (BoxesGroup*)mParentBox;
        if(mIsOutline) {
            groupBox->removeOutlinePathEffect(effect);
        } else if(mIsFill) {
            groupBox->removeFillPathEffect(effect);
        } else {
            groupBox->removePathEffect(effect);
        }
    }
}

bool PathEffectAnimators::hasEffects() {
    return !ca_mChildAnimators.isEmpty();
}

void PathEffectAnimators::filterPathForRelFrameBeforeThickness(
                                                const int &relFrame,
                                                SkPath *srcDstPath) {
    SkPath dstPath = *srcDstPath;
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        PathEffect *effectT = (PathEffect*)effect.data();
        if(effectT->applyBeforeThickness() && effectT->isVisible()) {
            SkPath srcPath = dstPath;
            effectT->filterPathForRelFrame(relFrame,
                                           srcPath,
                                           &dstPath, NULL);
        }
    }
    *srcDstPath = dstPath;
}

void PathEffectAnimators::filterPathForRelFrame(const int &relFrame,
                                                SkPath *srcDstPath,
                                                const bool &groupPathSum) {
    SkPath dstPath = *srcDstPath;
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        PathEffect *effectT = (PathEffect*)effect.data();
        if(effectT->applyBeforeThickness() || !effectT->isVisible()) {
            continue;
        }
        SkPath srcPath = dstPath;
        effectT->filterPathForRelFrame(relFrame,
                                       srcPath,
                                       &dstPath,
                                       groupPathSum);
    }
    *srcDstPath = dstPath;
}

void PathEffectAnimators::filterPathForRelFrameUntilGroupSum(
                        const int &relFrame,
                        SkPath *srcDstPath) {
    SkPath dstPath = *srcDstPath;
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        PathEffect *effectT = (PathEffect*)effect.data();
        if(effectT->applyBeforeThickness() || !effectT->isVisible()) {
            continue;
        }
        if(effectT->getEffectType() == GROUP_SUM_PATH_EFFECT) {
            break;
        }
        SkPath srcPath = dstPath;
        effectT->filterPathForRelFrame(relFrame,
                                       srcPath,
                                       &dstPath,
                                       false);
    }
    *srcDstPath = dstPath;
}


void PathEffectAnimators::filterPathForRelFrameBeforeThicknessF(
                                                const qreal &relFrame,
                                                SkPath *srcDstPath) {
    SkPath dstPath = *srcDstPath;
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        PathEffect *effectT = (PathEffect*)effect.data();
        if(effectT->applyBeforeThickness() && effectT->isVisible()) {
            SkPath srcPath = dstPath;
            effectT->filterPathForRelFrameF(relFrame,
                                           srcPath,
                                           &dstPath, NULL);
        }
    }
    *srcDstPath = dstPath;
}

void PathEffectAnimators::filterPathForRelFrameF(const qreal &relFrame,
                                                SkPath *srcDstPath,
                                                const bool &groupPathSum) {
    SkPath dstPath = *srcDstPath;
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        PathEffect *effectT = (PathEffect*)effect.data();
        if(effectT->applyBeforeThickness() || !effectT->isVisible()) {
            continue;
        }
        SkPath srcPath = dstPath;
        effectT->filterPathForRelFrameF(relFrame,
                                       srcPath,
                                       &dstPath,
                                       groupPathSum);
    }
    *srcDstPath = dstPath;
}

void PathEffectAnimators::filterPathForRelFrameUntilGroupSumF(
                        const qreal &relFrame,
                        SkPath *srcDstPath) {
    SkPath dstPath = *srcDstPath;
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        PathEffect *effectT = (PathEffect*)effect.data();
        if(effectT->applyBeforeThickness() || !effectT->isVisible()) {
            continue;
        }
        if(effectT->getEffectType() == GROUP_SUM_PATH_EFFECT) {
            break;
        }
        SkPath srcPath = dstPath;
        effectT->filterPathForRelFrameF(relFrame,
                                       srcPath,
                                       &dstPath,
                                       false);
    }
    *srcDstPath = dstPath;
}
