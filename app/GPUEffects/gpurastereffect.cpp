#include "gpurastereffect.h"
class GPUEffectAnimators;

GpuEffect::GpuEffect(const QString &name, const GpuEffectType type) :
    StaticComplexAnimator(name), mType(type) {}

void GpuEffect::writeIdentifier(QIODevice * const dst) const {
    dst->write(rcConstChar(&mType), sizeof(GpuEffectType));
}
