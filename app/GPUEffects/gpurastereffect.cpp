#include "gpurastereffect.h"
#include "exceptions.h"
#include "Animators/qrealanimator.h"
#include "Animators/gpueffectanimators.h"

GPURasterEffect::GPURasterEffect(const GPURasterEffectProgram * const program,
                                 const QString& name) :
    ComplexAnimator(name), mProgram(program) {

}
