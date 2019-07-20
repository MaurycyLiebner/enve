#include "custompatheffect.h"
#include "basicreadwrite.h"

CustomPathEffect::CustomPathEffect(const QString &name) :
    PathEffect(name, PathEffectType::CUSTOM) {}

void CustomPathEffect::writeIdentifier(QIODevice * const dst) const {
    PathEffect::writeIdentifier(dst);
    getIdentifier().write(dst);
}
