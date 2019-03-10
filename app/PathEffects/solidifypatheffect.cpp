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

void SolidifyPathEffect::filterPathForRelFrame(const qreal &relFrame,
                                                const SkPath &src,
                                                SkPath *dst,
                                                const bool &) {
    qreal widthT = mDisplacement->getCurrentEffectiveValueAtRelFrame(relFrame);
    gSolidify(widthT, src, dst);
}
