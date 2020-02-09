#include "blendeffect.h"
#include "Boxes/pathbox.h"

BlendEffect::BlendEffect() : StaticComplexAnimator("blend effect") {
    mZIndex = enve::make_shared<IntAnimator>("z-index");
    mClipPath = enve::make_shared<BoxTargetProperty>("clip path");

    ca_addChild(mZIndex);
    ca_addChild(mClipPath);
}

bool BlendEffect::isPathValid() const {
    return clipPathSource();
}

int BlendEffect::zIndex(const qreal relFrame) const {
    return mZIndex->getEffectiveIntValue(relFrame);
}

SkPath BlendEffect::clipPath(const qreal relFrame) const {
    const auto target = clipPathSource();
    if(!target) return SkPath();
    const qreal absFrame = prp_relFrameToAbsFrameF(relFrame);
    const qreal tRelFrame = target->prp_absFrameToRelFrameF(absFrame);
    return target->getAbsolutePath(tRelFrame);
}

PathBox *BlendEffect::clipPathSource() const {
    const auto target = mClipPath->getTarget();
    if(!target) return nullptr;
    if(!target->SWT_isPathBox()) return nullptr;
    return static_cast<PathBox*>(target);
}
