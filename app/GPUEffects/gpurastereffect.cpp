#include "gpurastereffect.h"
#include "exceptions.h"
#include "Animators/qrealanimator.h"

GPURasterEffect::GPURasterEffect(
        const GPURasterEffectProgram &program,
        const QString& name) :
    ComplexAnimator(name), mProgram(program) {

}
