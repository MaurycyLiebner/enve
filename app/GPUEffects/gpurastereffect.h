#ifndef GPURASTEREFFECT_H
#define GPURASTEREFFECT_H
#include "Animators/staticcomplexanimator.h"
#include "shadereffectcreator.h"
class GPUEffectAnimators;

class GPURasterEffectCaller : public StdSelfRef {
public:
    virtual void render(QGL33c * const gl,
                        GpuRenderTools& renderTools,
                        GpuRenderData& data) = 0;
};

#endif // GPURASTEREFFECT_H
