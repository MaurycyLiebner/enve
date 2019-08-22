#include "RasterEffects/customrastereffect.h"
#include "basicreadwrite.h"

CustomRasterEffect::CustomRasterEffect(const QString &name,
                                       const HardwareSupport hwSupport,
                                       const bool hwInterchangeable) :
    RasterEffect(name, hwSupport, hwInterchangeable,
                 RasterEffectType::CUSTOM) {}

void CustomRasterEffect::writeIdentifier(eWriteStream& dst) const {
    RasterEffect::writeIdentifier(dst);
    getIdentifier().write(dst);
}
