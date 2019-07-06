#include "shadereffect.h"
#include "Animators/gpueffectanimators.h"

ShaderEffect::ShaderEffect(const ShaderEffectCreator * const creator,
                           const ShaderEffectProgram * const program,
                           const QString& name,
                           const QList<stdsptr<PropertyCreator>> &props) :
    StaticComplexAnimator(name), mProgram(program), mCreator(creator) {
    for(const auto& propC : props) {
        ca_addChildAnimator(propC->create());
    }
}

void ShaderEffect::writeIdentifier(QIODevice * const dst) const {
    mCreator->writeIdentifier(dst);
}

GPUEffectAnimators *ShaderEffect::getParentEffectAnimators() {
    return static_cast<GPUEffectAnimators*>(mParent_k.data());
}
