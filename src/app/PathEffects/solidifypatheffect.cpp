#include "solidifypatheffect.h"
#include "pathoperations.h"
#include "Boxes/pathbox.h"

SolidifyPathEffect::SolidifyPathEffect() :
    PathEffect("solidify effect", PathEffectType::SOLIDIFY) {
    mDisplacement = SPtrCreate(QrealAnimator)("displacement");
    mDisplacement->setValueRange(-999.999, 999.999);
    mDisplacement->setCurrentBaseValue(5);

    ca_addChildAnimator(mDisplacement);

    setPropertyForGUI(mDisplacement.get());
}

void SolidifyPathEffect::apply(const qreal relFrame,
                               const SkPath &src,
                               SkPath * const dst) {
    const qreal widthT = mDisplacement->getEffectiveValue(relFrame);
    gSolidify(widthT, src, dst);
}
