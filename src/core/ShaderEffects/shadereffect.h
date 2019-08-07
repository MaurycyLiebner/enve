#ifndef SHADEREFFECT_H
#define SHADEREFFECT_H
#include "RasterEffects/rastereffect.h"
#include "shadereffectcreator.h"
#include "Tasks/updatable.h"

class ShaderEffect : public RasterEffect {
    e_OBJECT
    ShaderEffect(const QString &name,
                 const ShaderEffectCreator * const creator,
                 const ShaderEffectProgram * const program,
                 const QList<stdsptr<PropertyCreator>>& props);
public:
    void writeIdentifier(QIODevice * const dst) const;

    stdsptr<RasterEffectCaller> getEffectCaller(const qreal relFrame) const;

    void updateIfUsesProgram(const ShaderEffectProgram * const program) {
        if(program == mProgram)
            prp_afterWholeInfluenceRangeChanged();
    }
private:
    const ShaderEffectProgram * const mProgram;
    const ShaderEffectCreator * const mCreator;
};

#endif // SHADEREFFECT_H
