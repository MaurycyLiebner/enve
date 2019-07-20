#include "GPUEffects/customgpueffect.h"
#include "basicreadwrite.h"

CustomGpuEffect::CustomGpuEffect(const QString &name) :
    GpuEffect(name, GpuEffectType::CUSTOM) {}

void CustomGpuEffect::writeIdentifier(QIODevice * const dst) const {
    GpuEffect::writeIdentifier(dst);
    getIdentifier().write(dst);
}
