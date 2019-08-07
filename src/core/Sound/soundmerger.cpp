#include "soundmerger.h"

void mergeData(const float * const & src,
               const SampleRange& srcRange,
               float * const & dst,
               const SampleRange& dstRange,
               int nSamples,
               QrealSnapshot::Iterator volIt) {
    nSamples = qMin(qMin(nSamples, dstRange.span()), srcRange.span());
    if(volIt.staticValue()) {
        const qreal vol = volIt.getValueAndProgress(1);
        for(int i = 0; i < nSamples; i++) {
            const int dstId = dstRange.fMin + i;
            const int srcId = srcRange.fMin + i;
            dst[dstId] += static_cast<float>(qreal(src[srcId])*vol);
        }
    } else {
        for(int i = 0; i < nSamples; i++) {
            const int dstId = dstRange.fMin + i;
            const int srcId = srcRange.fMin + i;
            const qreal vol = volIt.getValueAndProgress(1);
            dst[dstId] += static_cast<float>(qreal(src[srcId])*vol);
        }
    }
}

void SoundMerger::process() {
    mSamples = enve::make_shared<Samples>(mSampleRange);
    const auto& dst = mSamples->fData;
    const int dstSamples = mSamples->fSampleRange.span();
    memset(dst, 0, ulong(dstSamples)*sizeof(float));
    for(const auto& sound : mSounds) {
        const auto& srcSamples = sound.fSamples;
        const qreal& stretch = sound.fStretch;

        const SampleRange smplsRelRange = srcSamples->fSampleRange;
        const SampleRange smplsSpeedRelRange{qRound(smplsRelRange.fMin*stretch),
                                             qRound(smplsRelRange.fMax*stretch)};
        const SampleRange smplsAbsRange = smplsSpeedRelRange.shifted(sound.fSampleShift);
        const SampleRange srcAbsRange = smplsAbsRange*sound.fSSAbsRange;
        const SampleRange srcNeededAbsRange = srcAbsRange*mSampleRange;
        const int absToRel = -qRound(srcSamples->fSampleRange.fMin*stretch) - sound.fSampleShift;
        const SampleRange srcNeededRelRange = srcNeededAbsRange.shifted(absToRel);

        const SampleRange dstAbsRange = mSampleRange;
        const SampleRange dstNeededAbsRange = dstAbsRange*srcNeededAbsRange;
        const SampleRange dstRelRange = dstNeededAbsRange.shifted(-mSampleRange.fMin);

        if(!dstRelRange.isValid()) continue;
        if(!srcNeededRelRange.isValid()) continue;
        const int firstVolSample = dstNeededAbsRange.fMin - sound.fSampleShift;
        QrealSnapshot::Iterator volIt(firstVolSample, 1000, &sound.fVolume);
        if(isOne4Dec(stretch)) {
            const int nSamples = qMin(srcNeededRelRange.span(), dstRelRange.span());
            const float * const & src = srcSamples->fData;
            mergeData(src, srcNeededRelRange, dst, dstRelRange, nSamples, volIt);
        } else {
            const int sampleRate = qRound(SOUND_SAMPLERATE*stretch);
            struct SwrContext * swrContext = swr_alloc();
            av_opt_set_int(swrContext, "in_channel_count",  1, 0);
            av_opt_set_int(swrContext, "out_channel_count", 1, 0);
            av_opt_set_int(swrContext, "in_channel_layout",  AV_CH_LAYOUT_MONO, 0);
            av_opt_set_int(swrContext, "out_channel_layout", AV_CH_LAYOUT_MONO, 0);
            av_opt_set_int(swrContext, "in_sample_rate", SOUND_SAMPLERATE, 0);
            av_opt_set_int(swrContext, "out_sample_rate", sampleRate, 0);
            av_opt_set_sample_fmt(swrContext, "in_sample_fmt", AV_SAMPLE_FMT_FLT, 0);
            av_opt_set_sample_fmt(swrContext, "out_sample_fmt", AV_SAMPLE_FMT_FLT,  0);
            swr_init(swrContext);
            if(!swr_is_initialized(swrContext)) {
                RuntimeThrow("Resampler has not been properly initialized");
            }
            uint8_t* buffer = nullptr;
            const int res = av_samples_alloc(&buffer, nullptr, 1, sampleRate,
                                             AV_SAMPLE_FMT_FLT, 0);
            if(res < 0) RuntimeThrow("Resampling output buffer alloc failed");
            int nSamples =
                    swr_convert(swrContext, &buffer, sampleRate,
                                (const uint8_t**)&srcSamples->fData,
                                SOUND_SAMPLERATE);
            if(nSamples < 0) RuntimeThrow("Resampling failed");
            const float * const & src = reinterpret_cast<float*>(buffer);
            mergeData(src, srcNeededRelRange, dst, dstRelRange, nSamples, volIt);
            av_freep(&buffer);
        }
    }
}
