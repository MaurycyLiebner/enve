#include "examplepatheffect.h"

qsptr<CustomPathEffect> createEffect(const bool outlinePathEffect) {
    return SPtrCreate(ExamplePathEffect)(outlinePathEffect);
}

QString effectName() {
    return "Example";
}

QByteArray effectIdentifier() {
    return
// make sure identifier for every effect is unique by adding random string
    "waer9yv11r3gl10x1qtm"
// here specify your effect's name and version
    "example.0.0.0";
}

bool supports(const QByteArray &identifier) {
    return QString(identifier) == QString(effectIdentifier());
}
#include "Animators/qrealanimator.h"
ExamplePathEffect::ExamplePathEffect(const bool outlinePathEffect) :
    CustomPathEffect(effectName().toLower(), outlinePathEffect) {
    mInfluence = SPtrCreate(QrealAnimator)(0, 0, 1, 0.1, "influence");
    ca_addChildAnimator(mInfluence);
}

QByteArray ExamplePathEffect::getIdentifier() const {
    return effectIdentifier();
}

void ExamplePathEffect::read(const QByteArray& identifier,
                             QIODevice * const src) {
    Q_UNUSED(identifier);
    mInfluence->readProperty(src);
}

void ExamplePathEffect::write(QIODevice * const dst) const {
    mInfluence->writeProperty(dst);
}

void ExamplePathEffect::apply(const qreal &relFrame,
                              const SkPath &src,
                              SkPath * const dst) {
    const SkScalar infl = toSkScalar(mInfluence->getEffectiveValue(relFrame));
    const SkScalar invInf = 1 - infl;
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
