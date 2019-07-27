#ifndef EFFECTSUBTASKSPAWNER_H
#define EFFECTSUBTASKSPAWNER_H
#include "smartPointers/sharedpointerdefs.h"

struct BoxRenderData;
class RasterEffectCaller;

namespace EffectSubTaskSpawner {
    void sSpawn(const stdsptr<RasterEffectCaller>& effect,
                const stdsptr<BoxRenderData>& data);
};

#endif // EFFECTSUBTASKSPAWNER_H
