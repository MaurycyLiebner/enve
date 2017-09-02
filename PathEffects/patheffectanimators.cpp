#include "patheffectanimators.h"
#include "patheffect.h"
#include "Boxes/pathbox.h"
#include <QDebug>

PathEffectAnimators::PathEffectAnimators(const bool &isOutline,
                                         PathBox *parentPath) :
    ComplexAnimator() {
    mIsOutline = isOutline;
    mParentPath = parentPath;
}

void PathEffectAnimators::addEffect(PathEffect *effect) {
    if(mIsOutline) {
        mParentPath->addOutlinePathEffect(effect);
    } else {
        mParentPath->addPathEffect(effect);
    }
}

bool PathEffectAnimators::hasEffects() {
    return !ca_mChildAnimators.isEmpty();
}

bool PathEffectAnimators::SWT_isEffectAnimators() { return true; }

void PathEffectAnimators::filterPath(SkPath *srcDstPath) {
    SkPath dstPath = *srcDstPath;
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
        SkPath srcPath = dstPath;
        ((PathEffect*)effect.data())->filterPath(srcPath, &dstPath);
    }
    *srcDstPath = dstPath;
}

void PathEffectAnimators::filterPathForRelFrame(const int &relFrame,
                                                SkPath *srcDstPath) {
    SkPath dstPath = *srcDstPath;
    Q_FOREACH(const QSharedPointer<Property> &effect, ca_mChildAnimators) {
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
