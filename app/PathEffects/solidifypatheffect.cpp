#include "solidifypatheffect.h"
#include "pathoperations.h"
#include "Boxes/pathbox.h"

SolidifyPathEffect::SolidifyPathEffect(const bool &outlinePathEffect) :
    PathEffect("solidify effect", SOLIDIFY_PATH_EFFECT, outlinePathEffect) {
    mDisplacement = SPtrCreate(QrealAnimator)("displacement");
    mDisplacement->setValueRange(-999.999, 999.999);
    mDisplacement->setCurrentBaseValue(5);

    ca_addChildAnimator(mDisplacement);
}

void SolidifyPathEffect::apply(const qreal &relFrame,
                               const SkPath &src,
                               SkPath * const dst) {
    const qreal widthT = mDisplacement->getEffectiveValue(relFrame);
    gSolidify(widthT, src, dst);
}
