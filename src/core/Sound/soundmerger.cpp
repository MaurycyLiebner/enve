#include "soundmerger.h"

void mergeInterleavedData(const float* const src,
                          const SampleRange& srcRange,
                          float * const dst,
                          const SampleRange& dstRange,
                          const int nSamples,
                          QrealSnapshot::Iterator volIt,
                          const int nChannels) {
    int dstId = dstRange.fMin*nChannels;
    int srcId = srcRange.fMin*nChannels;
    if(volIt.staticValue()) {
        const float vol = static_cast<float>(volIt.getValueAndProgress(1));
        for(int i = 0; i < nSamples; i++) {
            for(int j = 0; j < nChannels; j++) {
                dst[dstId++] += src[srcId++]*vol;
            }
        }
    } else {
        for(int i = 0; i < nSamples; i++) {
            const float vol = static_cast<float>(volIt.getValueAndProgress(1));
            for(int j = 0; j < nChannels; j++) {
                dst[dstId++] += src[srcId++]*vol;
            }
        }
    }
}

void mergeData(uchar const * const * const src,
               const SampleRange& srcRange,
               uchar ** const dst,
               const SampleRange& dstRange,
               int nSamples,
               QrealSnapshot::Iterator volIt,
               const AVSampleFormat format,
               const int nChannels) {
    nSamples = qMin(qMin(nSamples, dstRange.span()), srcRange.span());
    if(format == AV_SAMPLE_FMT_FLT) {
        mergeInterleavedData(reinterpret_cast<const float*>(src[0]), srcRange,
                             reinterpret_cast<float*>(dst[0]), dstRange,
                             nSamples, volIt, nChannels);
    } else RuntimeThrow("Unsupported format " + av_get_sample_fmt_name(format));
}

void SoundMerger::process() {
    const int nChannels = mSettings.channelCount();
    mSamples = enve::make_shared<Samples>(mSampleRange,
                                          mSettings.fSampleRate,
                                          mSettings.fSampleFormat,
                                          mSettings.fChannelLayout);
    mSamples->zeroAll();
    const auto dst = mSamples->fData;
    for(const auto& sound : mSounds) {
        const auto srcSamples = sound.fSamples;
        const qreal stretch = sound.fStretch;

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

            const auto src = srcSamples->fData;
            mergeData(src, srcNeededRelRange, dst, dstRelRange,
                      nSamples, volIt, mSettings.fSampleFormat, nChannels);
        } else {
            const int srcSampleRate = mSettings.fSampleRate;
            const int dstSampleRate = qRound(mSettings.fSampleRate*stretch);

            const AVSampleFormat sampleFormat = mSettings.fSampleFormat;
            const uint64_t chLayout = mSettings.fChannelLayout;
            const int chCount = av_get_channel_layout_nb_channels(chLayout);

            struct SwrContext * swrContext = swr_alloc();
            av_opt_set_int(swrContext, "in_channel_count", chCount, 0);
            av_opt_set_int(swrContext, "out_channel_count", chCount, 0);
            av_opt_set_int(swrContext, "in_channel_layout", chLayout, 0);
            av_opt_set_int(swrContext, "out_channel_layout", chLayout, 0);
            av_opt_set_int(swrContext, "in_sample_rate", srcSampleRate, 0);
            av_opt_set_int(swrContext, "out_sample_rate", dstSampleRate, 0);
            av_opt_set_sample_fmt(swrContext, "in_sample_fmt", sampleFormat, 0);
            av_opt_set_sample_fmt(swrContext, "out_sample_fmt", sampleFormat,  0);
            swr_init(swrContext);
            if(!swr_is_initialized(swrContext))
                RuntimeThrow("Resampler has not been properly initialized");
            uint8_t** buffer = nullptr;
            const int res = av_samples_alloc_array_and_samples(
                        &buffer, nullptr, chCount, dstSampleRate,
                        sampleFormat, 0);
            if(res < 0) RuntimeThrow("Resampling output buffer alloc failed");
            const int nSamples =
                    swr_convert(swrContext, buffer, dstSampleRate,
                                const_cast<const uint8_t**>(srcSamples->fData),
                                srcSampleRate);
            swr_free(&swrContext);
            if(nSamples < 0) RuntimeThrow("Resampling failed");
            mergeData(buffer, srcNeededRelRange, dst, dstRelRange,
                      nSamples, volIt, mSettings.fSampleFormat, nChannels);
            av_freep(&buffer);
        }
    }
}
