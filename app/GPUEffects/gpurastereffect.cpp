#include "gpurastereffect.h"
#include "exceptions.h"
#include "Animators/qrealanimator.h"
#include "Animators/gpueffectanimators.h"
#include "gpurastereffectcreator.h"

GPURasterEffect::GPURasterEffect(const GPURasterEffectProgram * const program,
                                 const QString& name) :
    ComplexAnimator(name), mProgram(program) {

}

void GPURasterEffect::writeProperty(QIODevice * const target) const {
    for(const auto& anim : ca_mChildAnimators)
        anim->writeProperty(target);
}

void GPURasterEffect::readProperty(QIODevice * const target) {
    for(const auto& anim : ca_mChildAnimators)
        anim->readProperty(target);
}

void GPURasterEffect::writeType(QIODevice * const dst) const {
    const int nChildren = ca_mChildAnimators.count();
    dst->write(rcConstChar(&nChildren), sizeof(int));
    for(const auto& anim : ca_mChildAnimators) {
        PropertyType type;
        if(dynamic_cast<QrealAnimator*>(anim.get())) {
            type = PTYPE_FLOAT;
        } else if(dynamic_cast<IntAnimator*>(anim.get())) {
            type = PTYPE_INT;
        } else RuntimeThrow("Only QrealAnimator and IntAnimator supported");
        dst->write(rcConstChar(&type), sizeof(PropertyType));
    }
}

QList<PropertyType> GPURasterEffect::sReadType(QIODevice * const src) {
    QList<PropertyType> props;
    int nChildren;
    src->read(rcChar(&nChildren), sizeof(int));
    for(int i = 0; i < nChildren; i++) {
        PropertyType type;
        src->read(rcChar(&type), sizeof(PropertyType));
        props << type;
    }
    return props;
}
