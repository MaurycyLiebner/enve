#include "oileffect.h"

class OilEffectCaller : public RasterEffectCaller {
public:
    OilEffectCaller(const HardwareSupport hwSupport);

    void processGpu(QGL33 * const gl,
                    GpuRenderTools& renderTools);
    void processCpu(CpuRenderTools& renderTools,
                    const CpuRenderData &data);
};

OilEffect::OilEffect() :
    RasterEffect("blur", HardwareSupport::cpuOnly,
                 false, RasterEffectType::OIL) {

}
