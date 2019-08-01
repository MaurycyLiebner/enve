#include "RasterEffects/customrastereffect.h"
#include "basicreadwrite.h"

CustomRasterEffect::CustomRasterEffect(const QString &name) :
    RasterEffect(name, RasterEffectType::CUSTOM) {}

void CustomRasterEffect::writeIdentifier(QIODevice * const dst) const {
    RasterEffect::writeIdentifier(dst);
    getIdentifier().write(dst);
}
