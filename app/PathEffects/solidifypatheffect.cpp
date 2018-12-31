#include "solidifypatheffect.h"
#include "pathoperations.h"
#include "Boxes/pathbox.h"
SolidifyPathEffect::SolidifyPathEffect(const bool &outlinePathEffect) :
    PathEffect("solidify effect", SOLIDIFY_PATH_EFFECT, outlinePathEffect) {
    mDisplacement = SPtrCreate(QrealAnimator)("displacement");
    mDisplacement->qra_setValueRange(-999.999, 999.999);
    mDisplacement->qra_setCurrentValue(10.);

    ca_addChildAnimator(mDisplacement);
}

void SolidifyPathEffect::filterPathForRelFrame(const int &relFrame,
                                               const SkPath &src,
                                               SkPath *dst,
                                               const qreal &scale,
                                               const bool &) {
    qreal widthT = mDisplacement->getCurrentEffectiveValueAtRelFrame(relFrame)/scale;
    gSolidify(widthT, src, dst);
}

void SolidifyPathEffect::filterPathForRelFrameF(const qreal &relFrame,
                                                const SkPath &src,
                                                SkPath *dst,
                                                const bool &) {
    qreal widthT = mDisplacement->getCurrentEffectiveValueAtRelFrameF(relFrame);
    gSolidify(widthT, src, dst);
}
