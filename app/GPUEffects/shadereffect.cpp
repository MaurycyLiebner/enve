#include "shadereffect.h"
#include "Animators/gpueffectanimators.h"

ShaderEffect::ShaderEffect(const QString& name,
                           const ShaderEffectCreator * const creator,
                           const ShaderEffectProgram * const program,
                           const QList<stdsptr<PropertyCreator>> &props) :
    GpuEffect(name, GpuEffectType::CUSTOM_SHADER),
    mProgram(program), mCreator(creator) {
    for(const auto& propC : props)
        ca_addChildAnimator(propC->create());
}

void ShaderEffect::writeIdentifier(QIODevice * const dst) const {
    GpuEffect::writeIdentifier(dst);
    mCreator->writeIdentifier(dst);
}
