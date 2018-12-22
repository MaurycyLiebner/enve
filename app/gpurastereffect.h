#ifndef GPURASTEREFFECT_H
#define GPURASTEREFFECT_H
#include "Animators/complexanimator.h"
#include "glhelpers.h"
class QDomElement;

struct GPURasterEffectCreator : public PropertyCreator {
    GPURasterEffectCreator(const QString& name) : PropertyCreator(name) {}
    QList<stdsptr<PropertyCreator>> fProperties;
    GLuint fProgram;
    qsptr<Property> create() const;

    static stdsptr<GPURasterEffectCreator> sLoadFromFile(
            QGL33c * const gl,
            const QString& filePath);
    static QList<stdsptr<GPURasterEffectCreator>> sEffectCreators;
};

class GPURasterEffect : public ComplexAnimator {
    friend class SelfRef;
public:

private:
    GPURasterEffect(const QString &name);
};
#endif // GPURASTEREFFECT_H
