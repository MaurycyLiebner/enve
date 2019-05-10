#include "soundmerger.h"
QDebug operator<< (QDebug d, const SampleRange &model) {
    d << model.fMin << ',' << model.fMax;
    return d;
}
#define VAL_AND_NAME(var) qDebug() << #var << var;
void SoundMerger::processTask() {
    mSamples = SPtrCreate(Samples)(mSampleRange);
    const auto& dst = mSamples->fData;
    const int dstSamples = mSamples->fSampleRange.span();
    memset(dst, 0, ulong(dstSamples)*sizeof(float));
    for(const auto& sound : mSounds) {
        const auto& srcSamples = sound.fSamples;
        const qreal& speed = sound.fSpeed;

        const SampleRange smplsRelRange = srcSamples->fSampleRange;
        const SampleRange smplsSpeedRelRange{qRound(smplsRelRange.fMin/speed),
                                             qRound(smplsRelRange.fMax/speed)};
        const SampleRange smplsAbsRange = smplsSpeedRelRange.shifted(sound.fSampleShift);
        const SampleRange srcAbsRange = smplsAbsRange*sound.fSSAbsRange;
        const SampleRange srcNeededAbsRange = srcAbsRange*mSampleRange;
        const int absToRel = -qRound(srcSamples->fSampleRange.fMin/speed) - sound.fSampleShift;
        const SampleRange srcNeededRelRange = srcNeededAbsRange.shifted(absToRel);

        const SampleRange dstAbsRange = mSampleRange;
        const SampleRange dstNeededAbsRange = dstAbsRange*srcNeededAbsRange;
        const SampleRange dstRelRange = dstNeededAbsRange.shifted(-mSampleRange.fMin);

//        VAL_AND_NAME(smplsRelRange);
//        VAL_AND_NAME(smplsSpeedRelRange);
//        VAL_AND_NAME(smplsAbsRange);
//        VAL_AND_NAME(srcAbsRange);
//        VAL_AND_NAME(srcNeededAbsRange);
//        VAL_AND_NAME(srcNeededRelRange);
//        VAL_AND_NAME(dstAbsRange);
//        VAL_AND_NAME(dstNeededAbsRange);
//        VAL_AND_NAME(dstRelRange);

        if(!dstRelRange.isValid()) continue;
        if(!srcNeededRelRange.isValid()) continue;

        if(isOne4Dec(speed)) {
            const int nSamples = qMin(srcNeededRelRange.span(), dstRelRange.span());
            float * const & src = srcSamples->fData;
            for(int i = 0; i < nSamples; i++) {
                const int dstId = dstRelRange.fMin + i;
                const int srcId = srcNeededRelRange.fMin + i;
                dst[dstId] += src[srcId];
            }
        } else {
            const int sampleRate = qRound(SOUND_SAMPLERATE/speed);
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
            float *buffer;
            const int res = av_samples_alloc((uint8_t**)&buffer,
                                             nullptr, 1, sampleRate,
                                             AV_SAMPLE_FMT_FLT, 0);
            if(res < 0) RuntimeThrow("Resampling output buffer alloc failed");
            int nSamples =
                    swr_convert(swrContext,
                                (uint8_t**)(&buffer),
                                sampleRate,
                                (const uint8_t**)&srcSamples->fData,
                                SOUND_SAMPLERATE);
            if(nSamples < 0) RuntimeThrow("Resampling failed");
            nSamples = qMin(nSamples, dstRelRange.span());
            nSamples = qMin(nSamples, srcNeededRelRange.span());
            float * const & src = buffer;
            for(int i = 0; i < nSamples; i++) {
                const int dstId = dstRelRange.fMin + i;
                const int srcId = srcNeededRelRange.fMin + i;
                dst[dstId] += src[srcId];
            }
            av_freep(&((uint8_t**)(&buffer))[0]);
        }
    }
}
