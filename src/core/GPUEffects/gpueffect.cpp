#include "gpueffect.h"

GpuEffect::GpuEffect(const QString &name, const GpuEffectType type) :
    eEffect(name), mType(type) {}

void GpuEffect::writeIdentifier(QIODevice * const dst) const {
    dst->write(rcConstChar(&mType), sizeof(GpuEffectType));
}
