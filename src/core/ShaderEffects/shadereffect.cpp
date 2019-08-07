#include "shadereffect.h"
#include "Animators/rastereffectanimators.h"
#include "shadereffectcaller.h"

ShaderEffect::ShaderEffect(const QString& name,
                           const ShaderEffectCreator * const creator,
                           const ShaderEffectProgram * const program,
                           const QList<stdsptr<ShaderPropertyCreator>> &props) :
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
    QJSEngine engine;

    UniformSpecifiers uniformSpecifiers;
    const int argsCount = mProgram->fPropUniLocs.count();
    for(int i = 0; i < argsCount; i++) {
        const GLint loc = mProgram->fPropUniLocs.at(i);
        const auto prop = ca_getChildAt(i);
        const auto& uniformC = mProgram->fPropUniCreators.at(i);
        uniformC->evaluate(engine, prop, relFrame);
        uniformSpecifiers << uniformC->create(loc, prop, relFrame);
    }
    const int valsCount = mProgram->fValueHandlers.count();
    for(int i = 0; i < valsCount; i++) {
        const GLint loc = mProgram->fValueLocs.at(i);
        const auto& value = mProgram->fValueHandlers.at(i);
        value->evaluate(engine);
        uniformSpecifiers << value->create(loc);
    }
    QMargins margin;
    if(!mProgram->fMarginScript.isEmpty()) {
        const auto jsVal = engine.evaluate(mProgram->fMarginScript);
        if(!jsVal.isNumber()) RuntimeThrow("Invalid Margin script result type '" +
                                           mProgram->fMarginScript + "'");
        else margin += qCeil(jsVal.toNumber());
    }
    return enve::make_shared<ShaderEffectCaller>(margin, *mProgram, uniformSpecifiers);
}
