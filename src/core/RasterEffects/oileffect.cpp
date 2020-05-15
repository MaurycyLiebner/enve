#include "oileffect.h"

#include "OilImpl/oilsimulator.h"

OilEffect::OilEffect() :
    RasterEffect("oil painting", HardwareSupport::cpuOnly,
                 false, RasterEffectType::OIL) {

}

QMargins OilEffect::getMargin() const {
    return QMargins() + 20;
}

class OilEffectCaller : public RasterEffectCaller {
public:
    OilEffectCaller(const HardwareSupport hwSupport) :
        RasterEffectCaller(hwSupport) {}

    int cpuThreads(const int available, const int area) const {
        Q_UNUSED(available) Q_UNUSED(area)
        return 1;
    }

    void processCpu(CpuRenderTools& renderTools,
                    const CpuRenderData &data) {
        Q_UNUSED(data);
        OilSimulator simulator(renderTools.fDstBtmp, false, false);

        simulator.setImage(renderTools.fSrcBtmp, true);

        while(!simulator.isFinished()) {
            simulator.update(false);
        }
    }
};

stdsptr<RasterEffectCaller> OilEffect::getEffectCaller(
        const qreal relFrame, const qreal resolution,
        const qreal influence, BoxRenderData* const data) const {
    Q_UNUSED(relFrame)
    Q_UNUSED(resolution)
    Q_UNUSED(influence)
    Q_UNUSED(data)
    return enve::make_shared<OilEffectCaller>(instanceHwSupport());
}
