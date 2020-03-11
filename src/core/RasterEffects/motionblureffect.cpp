// enve - 2D animations software
// Copyright (C) 2016-2020 Maurycy Liebner

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "motionblureffect.h"

#include "Boxes/boxrenderdata.h"
#include "Animators/qrealanimator.h"
#include "Boxes/boundingbox.h"

class MotionBlurCaller : public RasterEffectCaller {
    e_OBJECT
    friend class StdSelfRef;
    MotionBlurCaller(const HardwareSupport hwSupport,
                      const qreal sampleCount,
                      const qreal opacity,
                      const QList<stdsptr<BoxRenderData>>& samples) :
        RasterEffectCaller(hwSupport),
        mSampleCount(sampleCount),
        mOpacity(opacity),
        mSamples(samples) {}
public:
//    void processGpu(QGL33 * const gl,
//                    GpuRenderTools& renderTools);
    void processCpu(CpuRenderTools& renderTools,
                    const CpuRenderData &data);

    int cpuThreads(const int available, const int area) const {
        Q_UNUSED(available)
        Q_UNUSED(area)
        return 1;
    }

    bool srcDstSeparation() const { return false; }
private:
    const qreal mSampleCount;
    const qreal mOpacity;
    const QList<stdsptr<BoxRenderData>> mSamples;
};

MotionBlurEffect::MotionBlurEffect() :
    RasterEffect("motion blur", HardwareSupport::cpuOnly,
                 false, RasterEffectType::MOTION_BLUR) {
    mOpacity = enve::make_shared<QrealAnimator>("opacity");
    mOpacity->setValueRange(0, 999);
    mOpacity->setCurrentBaseValue(100);

    mNumberSamples = enve::make_shared<QrealAnimator>("samples count");
    mNumberSamples->setValueRange(0, 99);
    mNumberSamples->setCurrentBaseValue(1);

    mFrameStep = enve::make_shared<QrealAnimator>("frame step");
    mFrameStep->setValueRange(-999, 999);
    mFrameStep->setCurrentBaseValue(1);

    ca_addChild(mOpacity);
    ca_addChild(mNumberSamples);
    ca_addChild(mFrameStep);

    connect(this, &Property::prp_parentChanged,
            this, [this]() {
        mParentBox = getFirstAncestor<BoundingBox>();
    });
}

stdsptr<RasterEffectCaller> MotionBlurEffect::getEffectCaller(
            const qreal relFrame, const qreal resolution,
            const qreal influence, BoxRenderData * const data) const {
    if(mBlocked) return nullptr;
    Q_UNUSED(resolution)
    mBlocked = true;
    const auto idRange = mParentBox->prp_getIdenticalRelRange(relFrame);
    qreal sampleCount = mNumberSamples->getEffectiveValue(relFrame)*influence;
    const qreal opacity = mOpacity->getEffectiveValue(relFrame)*0.01*influence;
    const qreal frameStep = mFrameStep->getEffectiveValue(relFrame);
    if(isZero4Dec(frameStep)) return nullptr;

    const int nSamples = qCeil(sampleCount);
    if(nSamples == 0) return nullptr;
    qreal sampleRelFrame = relFrame - nSamples*frameStep;
    QList<stdsptr<BoxRenderData>> samples;
    for(int i = 0; i < nSamples; i++) {
        if(!idRange.inRange(sampleRelFrame)) {
            const auto sampleRenderData = mParentBox->queExternalRender(sampleRelFrame);
            if(sampleRenderData) {
                if(sampleRenderData->finished()) {
                    data->fOtherGlobalRects << sampleRenderData->fGlobalRect;
                } else {
                    sampleRenderData->fMotionBlurTarget = data;
                    sampleRenderData->addDependent(data);
                }
                samples << sampleRenderData;
            }
        }

        sampleRelFrame += frameStep;
    }
    mBlocked = false;
    return enve::make_shared<MotionBlurCaller>(
                instanceHwSupport(), sampleCount, opacity, samples);
}

FrameRange MotionBlurEffect::getMotionBlurPropsIdenticalRange(const int relFrame) const {
    auto range = mParentBox->getMotionBlurIdenticalRange(relFrame, true);
    if(range == FrameRange::EMINMAX) return range;
    const qreal nSamples = mNumberSamples->getEffectiveValue(relFrame);
    const qreal frameStep = mFrameStep->getEffectiveValue(relFrame);
    const qreal marginF = nSamples*frameStep;
    const int margin = marginF > 0 ? qCeil(marginF) : qFloor(marginF);
    if(margin == 0) return range;
    const int positive = margin > 0 ? margin : 0;
    const int negative = margin < 0 ? -margin : 0;
    if(relFrame - range.fMin < positive ||
       range.fMax - relFrame < negative) return {relFrame, relFrame};
    if(range.fMin == FrameRange::EMIN) {
        range.fMax -= negative;
    } else if(range.fMax == FrameRange::EMAX) {
        range.fMin += positive;
    } else {
        range = {qMin(relFrame, range.fMin + positive),
                 qMax(relFrame, range.fMax - negative)};
    }
    return range;
}

FrameRange MotionBlurEffect::prp_getIdenticalRelRange(const int relFrame) const {
    const auto propsRange = getMotionBlurPropsIdenticalRange(relFrame);
    const auto effectRange = RasterEffect::prp_getIdenticalRelRange(relFrame);
    return propsRange*effectRange;
}

void replaceIfHigherAlpha(const int x0, const int y0,
                          const SkPixmap &dst,
                          const SkPixmap &src,
                          const qreal alpha) {
    uint8_t *dstD = static_cast<uint8_t*>(dst.writable_addr());
    const uint8_t *srcD = static_cast<const uint8_t*>(src.addr());
    const int dstRowWidth = dst.rowBytesAsPixels();
    const int srcRowWidth = src.rowBytesAsPixels();

    int dstId = (y0 * dstRowWidth + x0)*4;
    int srcId = 0;
    const int yMax = qMin(src.height(), dst.height() - y0);
    const int xMax = qMin(src.width(), dst.width() - x0);
    const int iDstYInc = qMax(0, dstRowWidth - xMax)*4;
    const int iSrcYInc = qMax(0, srcRowWidth - xMax)*4;
    for(int y = 0; y < yMax; y++) {
        for(int x = 0; x < xMax; x++) {
            const int maxSrcId = src.width()*src.height()*4;
            Q_ASSERT(srcId + 3 < maxSrcId);
            const int maxDstId = dst.width()*dst.height()*4;
            Q_ASSERT(dstId + 3 < maxDstId);
            uchar& dstAlpha = dstD[dstId + 3];
            const uchar srcAlpha = static_cast<uchar>(qRound(srcD[srcId + 3]*alpha));
            if(dstAlpha < srcAlpha) {
                dstD[dstId] = static_cast<uint8_t>(qRound(srcD[srcId]*alpha));
                dstD[dstId + 1] = static_cast<uint8_t>(qRound(srcD[srcId + 1]*alpha));
                dstD[dstId + 2] = static_cast<uint8_t>(qRound(srcD[srcId + 2]*alpha));
                dstAlpha = srcAlpha;
            }
            dstId += 4;
            srcId += 4;
        }
        dstId += iDstYInc;
        srcId += iSrcYInc;
    }
}

void MotionBlurCaller::processCpu(CpuRenderTools& renderTools,
                                   const CpuRenderData &data) {
    const auto& bitmap = renderTools.fSrcBtmp;
    SkPixmap pixmap;
    if(!bitmap.peekPixels(&pixmap)) return;
    const qreal opacityStep = 1/(mSampleCount + 1);
    qreal sampleOpacity = opacityStep*(1 - qCeil(mSampleCount) + mSampleCount);
    for(const auto& sample : mSamples) {
        const qreal sampleAlpha = qBound(0., sampleOpacity*sampleOpacity*mOpacity, 1.);
        sampleOpacity += opacityStep;
        const auto& srcImg = sample->fRenderedImage;
        if(!srcImg) continue;
        const auto rasterImg = srcImg->makeRasterImage();
        if(!rasterImg) continue;
        SkPixmap samplePixmap;
        if(!rasterImg->peekPixels(&samplePixmap)) continue;
        const QPoint drawPos = sample->fGlobalRect.topLeft() - data.fPos;
        replaceIfHigherAlpha(qMax(0, drawPos.x()), qMax(0, drawPos.y()),
                             pixmap, samplePixmap, sampleAlpha);
    }
}
