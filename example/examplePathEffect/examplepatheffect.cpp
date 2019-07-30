#include "examplepatheffect.h"

qsptr<CustomPathEffect> createNewestVersionEffect() {
    // Use default, most up to date, version
    return SPtrCreate(ExamplePathEffect000)();
}

qsptr<CustomPathEffect> createEffect(
        const CustomIdentifier &identifier) {
    Q_UNUSED(identifier);
    // Choose version based on identifier
    // if(identifier.fVersion == CustomIdentifier::Version{0, 0, 0})
    return SPtrCreate(ExamplePathEffect000)();
}

// Returned value must be unique, lets enve distinguish effects
QString effectId() {
    return "waer9yv11r3gl10x1qtm";
}

// Name of your effect used in UI
QString effectName() {
    return "Example";
}

// here specify your effect's most up to date version
CustomIdentifier::Version effectVersion() {
    return { 0, 0, 0 };
}

CustomIdentifier effectIdentifier() {
    return { effectId(), effectName(), effectVersion() };
}

bool supports(const CustomIdentifier &identifier) {
    if(identifier.fEffectId != effectId()) return false;
    if(identifier.fEffectName != effectName()) return false;
    return identifier.fVersion == effectVersion();
}

#include "enveCore/Animators/qrealanimator.h"
ExamplePathEffect000::ExamplePathEffect000() :
    CustomPathEffect(effectName().toLower()) {
    mInfluence = SPtrCreate(QrealAnimator)(0, 0, 1, 0.1, "influence");
    ca_addChild(mInfluence);
}

CustomIdentifier ExamplePathEffect000::getIdentifier() const {
    return { effectId(), effectName(), { 0, 0, 0 } };
}

void ExamplePathEffect000::apply(const qreal relFrame,
                                 const SkPath &src,
                                 SkPath * const dst) {
    const float infl = toSkScalar(mInfluence->getEffectiveValue(relFrame));
    const float invInf = 1 - infl;
    dst->reset();

    SkPath::Iter iter(src, false);
    SkPoint pts[4];
    for(;;) {
        switch(iter.next(pts, true, true)) {
        case SkPath::kLine_Verb: {
            dst->lineTo(pts[1]);
        } break;
        case SkPath::kQuad_Verb: {
            dst->quadTo(pts[1]*invInf + (pts[0] + pts[2])*0.5f*infl, pts[2]);
        } break;
        case SkPath::kConic_Verb: {
            dst->conicTo(pts[1]*invInf + (pts[0] + pts[2])*0.5f*infl,
                         pts[2], iter.conicWeight());
        } break;
        case SkPath::kCubic_Verb: {
            dst->cubicTo(pts[0]*infl + pts[1]*invInf,
                         pts[3]*infl + pts[2]*invInf, pts[3]);
        } break;
        case SkPath::kClose_Verb: {
            dst->close();
        } break;
        case SkPath::kMove_Verb: {
            dst->moveTo(pts[0]);
        } break;
        case SkPath::kDone_Verb:
            return;
        }
    }
}
