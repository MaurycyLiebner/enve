#include "shadereffect.h"
#include "Animators/rastereffectanimators.h"
#include "shadereffectcaller.h"

ShaderEffect::ShaderEffect(const QString& name,
                           const ShaderEffectCreator * const creator,
                           const ShaderEffectProgram * const program,
                           const QList<stdsptr<PropertyCreator>> &props) :
    RasterEffect(name, RasterEffectType::CUSTOM_SHADER),
    mProgram(program), mCreator(creator) {
    for(const auto& propC : props)
        ca_addChild(propC->create());
}

void ShaderEffect::writeIdentifier(QIODevice * const dst) const {
    RasterEffect::writeIdentifier(dst);
    mCreator->writeIdentifier(dst);
}

stdsptr<RasterEffectCaller> ShaderEffect::getEffectCaller(const qreal relFrame) const {
    UniformSpecifiers uniformSpecifiers;
    const int argsCount = mProgram->fArgumentLocs.count();
    for(int i = 0; i < argsCount; i++) {
        const GLint loc = mProgram->fArgumentLocs.at(i);
        const auto prop = ca_getChildAt(i);
        const auto& uniformC = mProgram->fUniformCreators.at(i);
        uniformSpecifiers << uniformC->create(loc, prop, relFrame);
    }
    const auto margin = getMarginAtRelFrame(relFrame);
    return SPtrCreate(ShaderEffectCaller)(margin, *mProgram, uniformSpecifiers);
}
