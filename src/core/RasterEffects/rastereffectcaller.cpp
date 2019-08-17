#include "rastereffectcaller.h"


RasterEffectCaller::RasterEffectCaller(const HardwareSupport hwSupport,
                                       const bool forceMargin,
                                       const QMargins &margin) :
    fForceMargin(forceMargin), fHwSupport(hwSupport), fMargin(margin) {}

RasterEffectCaller::RasterEffectCaller(const HardwareSupport hwSupport) :
    RasterEffectCaller(hwSupport, false, QMargins()) {}

void RasterEffectCaller::processGpu(QGL33 * const gl,
                                    GpuRenderTools &renderTools,
                                    GpuRenderData &data) {
    Q_UNUSED(gl);
    Q_UNUSED(renderTools);
    Q_UNUSED(data);
}

void RasterEffectCaller::processCpu(CpuRenderTools &renderTools,
                                    const CpuRenderData &data) {
    Q_UNUSED(renderTools);
    Q_UNUSED(data);
}

int RasterEffectCaller::cpuThreads(const int available,
                                   const int area) const {
    return qMin(area/(150*150) + 1, available);
}
#include "skia/skqtconversions.h"
void RasterEffectCaller::setSrcRect(const SkIRect &srcRect,
                                    const SkIRect &clampRect) {
    const int sl = srcRect.left();
    const int st = srcRect.top();
    const int sr = srcRect.right();
    const int sb = srcRect.bottom();

    const int ml = fMargin.left();
    const int mt = fMargin.top();
    const int mr = fMargin.right();
    const int mb = fMargin.bottom();

    const int l = sl - ml;
    const int t = st - mt;
    const int r = sr + mr;
    const int b = sb + mb;

    const int cl = clampRect.left();
    const int ct = clampRect.top();
    const int cr = clampRect.right();
    const int cb = clampRect.bottom();

    if(fForceMargin) {
        fSrcRect = SkIRect::MakeLTRB(qMax(l, cl - ml),
                                     qMax(t, ct - mt),
                                     qMin(r, cr + mr),
                                     qMin(b, cb + mb));
        fDstRect = fSrcRect;
    } else {
        fSrcRect = srcRect;
        fDstRect = SkIRect::MakeLTRB(qMax(l, cl), qMax(t, ct),
                                     qMin(r, cr), qMin(b, cb));
    }
}
