#include "oileffect.h"

#include "Animators/qrealanimator.h"
#include "OilImpl/oilsimulator.h"

OilEffect::OilEffect() :
    RasterEffect("oil painting", HardwareSupport::cpuOnly,
                 false, RasterEffectType::OIL) {
    mBrushSize = enve::make_shared<QPointFAnimator>(
        QPointF{16., 64.}, QPointF{4., 4.},
        QPointF{999.999, 999.999}, QPointF{1., 1.},
        "min", "max", "brush size");
    ca_addChild(mBrushSize);
    connect(mBrushSize->getYAnimator(),
            &QrealAnimator::effectiveValueChanged,
            this, &RasterEffect::forcedMarginChanged);
    mAccuracy = enve::make_shared<QrealAnimator>(0.7, 0, 1, 0.01, "accuracy");
    ca_addChild(mAccuracy);
}

QMargins OilEffect::getMargin() const {
    return QMargins() + qCeil(2*mBrushSize->getEffectiveYValue());
}

class OilEffectCaller : public RasterEffectCaller {
public:
    OilEffectCaller(const QPointF& brushSize,
                    const qreal accuracy,
                    const HardwareSupport hwSupport) :
        RasterEffectCaller(hwSupport),
        mMinBrushSize(brushSize.x()),
        mMaxBrushSize(brushSize.y()),
        mAccuracy(accuracy) {}

    int cpuThreads(const int available, const int area) const {
        Q_UNUSED(available) Q_UNUSED(area)
        return 1;
    }

    void processCpu(CpuRenderTools& renderTools,
                    const CpuRenderData &data) {
        Q_UNUSED(data);
        OilSimulator simulator(renderTools.fDstBtmp, false, false);
        simulator.SMALLER_BRUSH_SIZE = mMinBrushSize;
        simulator.BIGGER_BRUSH_SIZE = mMaxBrushSize;
        const int accVal = 100*(1 - 0.7*mAccuracy);
        simulator.MAX_COLOR_DIFFERENCE = {accVal, accVal, accVal};

        simulator.setImage(renderTools.fSrcBtmp, true);

        while(!simulator.isFinished()) {
            simulator.update(false);
        }
    }

private:
    const qreal mMinBrushSize;
    const qreal mMaxBrushSize;
    const qreal mAccuracy;
};

stdsptr<RasterEffectCaller> OilEffect::getEffectCaller(
        const qreal relFrame, const qreal resolution,
        const qreal influence, BoxRenderData* const data) const {
    Q_UNUSED(data)
    Q_UNUSED(influence)
    const QPointF size = mBrushSize->getEffectiveValue(relFrame)*resolution;
    const qreal acc = mAccuracy->getEffectiveValue(relFrame);
    return enve::make_shared<OilEffectCaller>(size, acc, instanceHwSupport());
}
