#include "soundreader.h"
#include "audiostreamsdata.h"
#include "CacheHandlers/soundcachehandler.h"
#include "CacheHandlers/soundcachecontainer.h"
#include "Sound/soundcomposition.h"

void SoundReader::afterProcessing() {
    mCacheHandler->secondReaderFinished(mSecondId, mSamples);
}

void SoundReader::afterCanceled() {
    mCacheHandler->secondReaderCanceled(mSecondId);
}

void seek(const int tryN, const int secondId,
          AVFormatContext * const formatContext,
          const int audioStreamIndex, AVStream * const audioStream,
          AVCodecContext * const codecContext) {
    const int64_t tsms = qFloor(secondId * 1000 - tryN);
    const int64_t tm = av_rescale(tsms, audioStream->time_base.den,
                                  audioStream->time_base.num)/1000;
    if(tm <= 0)
        avformat_seek_file(formatContext, audioStreamIndex,
                           INT64_MIN, 0, 0, 0);
    else {
        const int64_t tsms0 = qFloor((secondId - tryN) * 1000);
        const int64_t tm0 = av_rescale(tsms0, audioStream->time_base.den,
                                       audioStream->time_base.num)/1000;
        if(avformat_seek_file(formatContext, audioStreamIndex, tm0,
                              tm, tm, AVSEEK_FLAG_FRAME) < 0) {
            avformat_seek_file(formatContext, audioStreamIndex,
                               INT64_MIN, 0, INT64_MAX, 0);
        }
    }
    avcodec_flush_buffers(codecContext);
}

void SoundReader::readFrame() {
    if(!mOpenedAudio->fOpened)
        RuntimeThrow("Cannot read frame from closed AudioStream");
    const auto formatContext = mOpenedAudio->fFormatContext;
    const auto audioStreamIndex = mOpenedAudio->fAudioStreamIndex;
    const auto audioStream = mOpenedAudio->fAudioStream;
    const auto codecPars = audioStream->codecpar;
    const int srcSampleRate = codecPars->sample_rate;
    const qreal dstSamplesPerSrc = SOUND_SAMPLERATE/qreal(srcSampleRate);
    const auto packet = mOpenedAudio->fPacket;
    const auto decodedFrame = mOpenedAudio->fDecodedFrame;
    const auto codecContext = mOpenedAudio->fCodecContext;
    const auto swrContext = mOpenedAudio->fSwrContext;

    const int firstSample = mSecondId*SOUND_SAMPLERATE;

    int seekTry = 0;
    if(mOpenedAudio->fLastDstSample >= firstSample ||
       firstSample - mOpenedAudio->fLastDstSample > SOUND_SAMPLERATE) {
        seek(seekTry++, mSecondId, formatContext,
             audioStreamIndex, audioStream, codecContext);
    }

    bool firstFrame = true;
    int currentDstSample = 0;
    float * audioData = nullptr;
    SampleRange audioDataRange{mSampleRange.fMin, mSampleRange.fMin - 1};
    int nSamples = 0;
    while(true) {
        mOpenedAudio->fLastDstSample = -10*SOUND_SAMPLERATE;
        if(av_read_frame(formatContext, packet) < 0) {
            break;
        } else {
            if(packet->stream_index == audioStreamIndex) {
                const int sendRet = avcodec_send_packet(codecContext, packet);
                if(sendRet < 0) RuntimeThrow("Sending packet to the decoder failed");

                const int recRet = avcodec_receive_frame(codecContext, decodedFrame);
                av_packet_unref(packet);

                if(recRet == AVERROR_EOF)
                    break;
                else if(recRet == AVERROR(EAGAIN)) {
                    continue;
                } else if(recRet < 0)
                    RuntimeThrow("Did not receive frame from the decoder");
            } else {
                av_packet_unref(packet);
                continue;
            }
        }

        // calculate PTS:
        if(firstFrame) {
            int64_t pts = av_frame_get_best_effort_timestamp(decodedFrame);
            pts = av_rescale_q(pts, audioStream->time_base, AV_TIME_BASE_Q);
            currentDstSample = static_cast<int>(pts*SOUND_SAMPLERATE/1000000);
            if(currentDstSample > firstSample) {
                if(seekTry > 3) {
                    qDebug() << "sample" << QString::number(currentDstSample) +
                                " instead of " + QString::number(firstSample);
                } else {
                    av_frame_unref(decodedFrame);
                    seek(seekTry++, mSecondId, formatContext,
                         audioStreamIndex, audioStream, codecContext);
                    continue;
                }
            }
        }

        if(currentDstSample + decodedFrame->nb_samples >= mSecondId*SOUND_SAMPLERATE) {
            // resample frames
            uint8_t* buffer = nullptr;
            const int bufferSamples = qCeil(decodedFrame->nb_samples*dstSamplesPerSrc);
            const int res = av_samples_alloc(&buffer, nullptr, 1,
                                             bufferSamples, AV_SAMPLE_FMT_FLT, 0);
            if(res < 0) RuntimeThrow("Resampling output buffer alloc failed");

            const int nDstSamples =
                    swr_convert(swrContext, &buffer, bufferSamples,
                                (const uint8_t**)decodedFrame->data,
                                decodedFrame->nb_samples);
            if(nSamples < 0) RuntimeThrow("Resampling failed");
            // append resampled frames to data
            const SampleRange frameSampleRange{currentDstSample, currentDstSample + nDstSamples - 1};
            const SampleRange neededSampleRange = mSampleRange*frameSampleRange;
            const int firstRelSample = neededSampleRange.fMin - frameSampleRange.fMin;
            const int nSamplesInRange = neededSampleRange.span();
            if(nSamplesInRange > 0) {
                const int newNSamples = nSamples + nSamplesInRange;
                const ulong newAudioDataSize = static_cast<ulong>(newNSamples) * sizeof(float);
                void * const audioDataMem = realloc(audioData, newAudioDataSize);
                audioData = static_cast<float*>(audioDataMem);
                const auto src = reinterpret_cast<float*>(buffer) + firstRelSample;
                float * const dst = audioData + nSamples;
                memcpy(dst, src, static_cast<ulong>(nSamplesInRange) * sizeof(float));
                nSamples = newNSamples;
                if(firstFrame) audioDataRange = neededSampleRange;
                else audioDataRange += neededSampleRange;
            }

            av_freep(&buffer);
            firstFrame = false;

            currentDstSample += nDstSamples;
            mOpenedAudio->fLastDstSample = currentDstSample - 1;
        }

        if(currentDstSample >= mSampleRange.fMax) break;
        av_frame_unref(decodedFrame);
    }
    av_frame_unref(decodedFrame);
    mSamples = enve::make_shared<Samples>(audioData, audioDataRange);
}

#include "Sound/soundmerger.h"
void SoundReaderForMerger::afterProcessing() {
    for(const auto& merger : mMergers) {
        if(!merger) continue;
        for(const auto& ss : mSSAbsRanges) {
            merger->addSoundToMerge({ss.fSampleShift, ss.fSamplesRange,
                                     ss.fVolume, ss.fSpeed,
                                     enve::make_shared<Samples>(getSamples())});
        }
    }
    SoundReader::afterProcessing();
}
