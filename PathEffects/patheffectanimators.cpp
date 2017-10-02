#include "patheffectanimators.h"
#include "patheffect.h"
#include "Boxes/pathbox.h"
#include <QDebug>

PathEffectAnimators::PathEffectAnimators(const bool &isOutline,
                                         const bool &isFill,
                                         PathBox *parentPath) :
    ComplexAnimator() {
    mIsOutline = isOutline;
    mIsFill = isFill;
    mParentPath = parentPath;
}

void PathEffectAnimators::addEffect(PathEffect *effect) {
    if(mIsOutline) {
        mParentPath->addOutlinePathEffect(effect);
    } else if(mIsFill) {
        mParentPath->addFillPathEffect(effect);
    } else {
        mParentPath->addPathEffect(effect);
    }
}

void PathEffectAnimators::removeEffect(PathEffect *effect) {
    if(mIsOutline) {
        mParentPath->removeOutlinePathEffect(effect);
    } else if(mIsFill) {
        mParentPath->removeFillPathEffect(effect);
    } else {
        mParentPath->removePathEffect(effect);
    }
}

bool PathEffectAnimators::hasEffects() {
    return !ca_mChildAnimators.isEmpty();
}

void PathEffectAnimators::filterPath(SkPath *srcDstPath) {
    SkPath dstPath = *srcDstPath;
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        SkPath srcPath = dstPath;
        ((PathEffect*)effect.data())->filterPath(srcPath, &dstPath);
    }
    *srcDstPath = dstPath;
}

void PathEffectAnimators::filterPathForRelFrameBeforeThickness(
                                                const int &relFrame,
                                                SkPath *srcDstPath) {
    SkPath dstPath = *srcDstPath;
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        if(((PathEffect*)effect.data())->applyBeforeThickness()) {
            SkPath srcPath = dstPath;
            ((PathEffect*)effect.data())->filterPathForRelFrame(relFrame,
                                                                srcPath,
                                                                &dstPath);
        }
    }
    *srcDstPath = dstPath;
}

void PathEffectAnimators::filterPathForRelFrame(const int &relFrame,
                                                SkPath *srcDstPath) {
    SkPath dstPath = *srcDstPath;
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        if(((PathEffect*)effect.data())->applyBeforeThickness()) {
            continue;
        }
        SkPath srcPath = dstPath;
        ((PathEffect*)effect.data())->filterPathForRelFrame(relFrame,
                                                            srcPath,
                                                            &dstPath);
    }
    *srcDstPath = dstPath;
}

//void PathEffectAnimators::makeDuplicate(Property *target) {
//    PathEffectAnimators *eaTarget = ((PathEffectAnimators*)target);
//    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
//        eaTarget->addEffect((PathEffect*)
//                    ((PathEffect*)effect.data())->makeDuplicate());
//    }
//}
