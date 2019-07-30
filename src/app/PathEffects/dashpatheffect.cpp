#include "dashpatheffect.h"
#include "Animators/qrealanimator.h"

DashPathEffect::DashPathEffect() :
    PathEffect("dash effect", PathEffectType::DASH) {
    mSize = SPtrCreate(QrealAnimator)("size");
    mSize->setValueRange(0.1, 9999.999);
    mSize->setCurrentBaseValue(5);

    ca_addChild(mSize);

    setPropertyForGUI(mSize.get());
}

void DashPathEffect::apply(const qreal relFrame,
                           const SkPath &src,
                           SkPath * const dst) {
    const auto width = toSkScalar(mSize->getEffectiveValue(relFrame));
    const float intervals[] = { width, width };
    SkStrokeRec rec(SkStrokeRec::kHairline_InitStyle);
    SkRect cullRec = src.getBounds();
    dst->reset();
    SkDashPathEffect::Make(intervals, 2, 0.f)->filterPath(dst, src, &rec, &cullRec);
}
