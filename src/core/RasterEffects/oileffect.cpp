#include "oileffect.h"

#include "Animators/qrealanimator.h"
#include "OilImpl/oilsimulator.h"

#define TIME_BEGIN const auto t1 = std::chrono::high_resolution_clock::now();
#define TIME_END(name) const auto t2 = std::chrono::high_resolution_clock::now(); \
                       const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count(); \
                       qDebug() << name << duration << "ms" << endl;

//#define OilEffect_TIMING

OilEffect::OilEffect() :
    RasterEffect("oil painting", HardwareSupport::cpuPreffered,
                 true, RasterEffectType::OIL) {
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

    mStrokeLength = enve::make_shared<QrealAnimator>(2.3, 0.2, 5, 0.01, "stroke length");
    ca_addChild(mStrokeLength);
    connect(mStrokeLength.get(),
            &QrealAnimator::effectiveValueChanged,
            this, &RasterEffect::forcedMarginChanged);

    mMaxStrokes = enve::make_shared<QrealAnimator>(100, 0, 9999.999, 1, "max strokes");
    ca_addChild(mMaxStrokes);
}

QMargins OilEffect::getMargin() const {
    return QMargins() + qCeil(mStrokeLength->getEffectiveValue()*
                              mBrushSize->getEffectiveYValue());
}

class OilEffectCaller : public RasterEffectCaller {
public:
    OilEffectCaller(const QPointF& brushSize,
                    const qreal accuracy,
                    const qreal strokeLength,
                    const qreal resolution,
                    const int maxStrokes,
                    const HardwareSupport hwSupport) :
        RasterEffectCaller(hwSupport),
        mMinBrushSize(brushSize.x()),
        mMaxBrushSize(brushSize.y()),
        mAccuracy(accuracy),
        mStrokeLength(strokeLength),
        mResolution(resolution),
        mMaxStrokes(maxStrokes) {}

    int cpuThreads(const int available, const int area) const {
        Q_UNUSED(available) Q_UNUSED(area)
        return 1;
    }

    void setupSimulator(OilSimulator& simulator) {
        simulator.SMALLER_BRUSH_SIZE = mMinBrushSize;
        simulator.BIGGER_BRUSH_SIZE = mMaxBrushSize;
        const int accVal = 100*(1 - 0.7*mAccuracy);
        simulator.MAX_COLOR_DIFFERENCE = {accVal, accVal, accVal};
        simulator.RELATIVE_TRACE_LENGTH = mStrokeLength;
        simulator.MIN_TRACE_LENGTH = 16*mResolution;
    }

    void processCpu(CpuRenderTools& renderTools,
                    const CpuRenderData &data) {
        Q_UNUSED(data);
        if(mMaxStrokes <= 0) return;
#ifdef OilEffect_TIMING
        TIME_BEGIN
#endif
        OilSimulator simulator(renderTools.fDstBtmp, false, false);
        setupSimulator(simulator);

        simulator.setImage(renderTools.fSrcBtmp, true);

        for(int i = 0; i < mMaxStrokes; i++) {
            simulator.update(false);
            if(simulator.isFinished()) break;
        }
#ifdef OilEffect_TIMING
        TIME_END("CPU Oil Painting")
#endif
    }

    void processGpu(QGL33 * const gl, GpuRenderTools &renderTools) {
        Q_UNUSED(gl)
        if(mMaxStrokes <= 0) return;
#ifdef OilEffect_TIMING
        TIME_BEGIN
#endif
        const auto& src = renderTools.getSrcTexture();
        auto srcBtmp = src.bitmapSnapshot(gl);

        const int width = srcBtmp.width();
        const int height = srcBtmp.height();
        for(int y = 0; y < height/2; y++) {
            const int invY = height - 1 - y;

            auto srcAddr1 = srcBtmp.getAddr32(0, invY);
            auto srcAddr2 = srcBtmp.getAddr32(0, y);

            for(int x = 0; x < width; x++) {
                std::swap(*srcAddr1++, *srcAddr2++);
            }
        }

        renderTools.switchToSkia();
        const auto canvas = renderTools.requestTargetCanvas();

        OilSimulator simulator(*canvas, false, false);
        setupSimulator(simulator);

        simulator.setImage(srcBtmp, true);

        for(int i = 0; i < mMaxStrokes; i++) {
            simulator.update(false);
            if(simulator.isFinished()) break;
        }

        canvas->flush();
        renderTools.swapTextures();
#ifdef OilEffect_TIMING
        TIME_END("GPU Oil Painting")
#endif
    }
private:
    const qreal mMinBrushSize;
    const qreal mMaxBrushSize;
    const qreal mAccuracy;
    const qreal mStrokeLength;
    const qreal mResolution;
    const int mMaxStrokes;
};

stdsptr<RasterEffectCaller> OilEffect::getEffectCaller(
        const qreal relFrame, const qreal resolution,
        const qreal influence, BoxRenderData* const data) const {
    Q_UNUSED(data)
    Q_UNUSED(influence)
    const QPointF size = mBrushSize->getEffectiveValue(relFrame)*resolution;
    const qreal acc = mAccuracy->getEffectiveValue(relFrame);
    const qreal len = mStrokeLength->getEffectiveValue(relFrame);
    const int maxStrokes = qRound(mMaxStrokes->getEffectiveValue(relFrame));
    return enve::make_shared<OilEffectCaller>(size, acc, len, resolution,
                                              maxStrokes, instanceHwSupport());
}
