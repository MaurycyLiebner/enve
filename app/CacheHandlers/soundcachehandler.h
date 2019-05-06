#ifndef SOUNDCACHEHANDLER_H
#define SOUNDCACHEHANDLER_H
#include "CacheHandlers/hddcachablecachehandler.h"
#include "CacheHandlers/soundcachecontainer.h"
#include "Decode/audiodecode.h"
#include "FileCacheHandlers/audiostreamsdata.h"
class SoundCacheHandler;

class SoundReader : public HDDTask {
    friend class StdSelfRef;
protected:
    SoundReader(SoundCacheHandler * const cacheHandler,
                AudioStreamsData * const openedAudio,
                const int& secondId, const SampleRange& sampleRange) :
        mCacheHandler(cacheHandler), mOpenedAudio(openedAudio),
        mSecondId(secondId), mSampleRange(sampleRange) {}

    void afterProcessing() {
        mCacheHandler->createNew<SoundCacheContainer>(mSamples, mSecondId);
    }

    void afterCanceled() {
        mCacheHandler->frameLoaderCanceled(mSecondId);
    }

public:
    void processTask() {
        readFrame();
    }
private:
    void readFrame() {
        if(!mOpenedAudio->fOpened)
            RuntimeThrow("Cannot read frame from closed VideoStream");
        const auto formatContext = mOpenedAudio->fFormatContext;
        const auto audioStreamIndex = mOpenedAudio->fAudioStreamIndex;
        const auto audioStream = mOpenedAudio->fAudioStream;
        const auto packet = mOpenedAudio->fPacket;
        const auto decodedFrame = mOpenedAudio->fDecodedFrame;
        const auto codecContext = mOpenedAudio->fCodecContext;
        const auto swrContext = mOpenedAudio->fSwrContext;
        const qreal fps = mOpenedAudio->fFps;

        const int64_t tsms = qFloor(mSecondId * 1000 - 1);
        const int64_t tm = av_rescale(tsms, audioStream->time_base.den,
                                      audioStream->time_base.num)/1000;
        if(tm <= 0)
            avformat_seek_file(formatContext, audioStreamIndex,
                               INT64_MIN, 0, 0, 0);
        else {
            const int64_t tsms0 = qFloor((mSecondId - 1) * 1000);
            const int64_t tm0 = av_rescale(tsms0, audioStream->time_base.den,
                                           audioStream->time_base.num)/1000;
            if(avformat_seek_file(formatContext, audioStreamIndex, tm0,
                                  tm, tm, AVSEEK_FLAG_FRAME) < 0) {
                qDebug() << "Failed to seek to " << mSecondId;
                avformat_seek_file(formatContext, audioStreamIndex,
                                   INT64_MIN, 0, INT64_MAX, 0);
            }
        }

        avcodec_flush_buffers(codecContext);
        int64_t pts = 0;
        bool firstSample = true;
        int currentSample = 0;
        float * audioData = nullptr;
        int nSamples = 0;
        while(true) {
            if(av_read_frame(formatContext, packet) < 0) {
                break;
            } else {
                if(packet->stream_index == audioStreamIndex) {
                    const int sendRet = avcodec_send_packet(codecContext, packet);
                    if(sendRet < 0) {
                        fprintf(stderr, "Sending packet to the decoder failed\n");
                        return;
                    }
                    const int recRet = avcodec_receive_frame(codecContext, decodedFrame);
                    if(recRet == AVERROR_EOF) {
                        return;
                    } else if(recRet == AVERROR(EAGAIN)) {
                        av_packet_unref(packet);
                        continue;
                    } else if(recRet < 0) {
                        fprintf(stderr, "Did not receive frame from the decoder\n");
                        return;
                    }
                    av_packet_unref(packet);
                } else {
                    av_packet_unref(packet);
                    continue;
                }
            }

            // calculate PTS:
            pts = av_frame_get_best_effort_timestamp(decodedFrame);
            pts = av_rescale_q(pts, audioStream->time_base, AV_TIME_BASE_Q);
            const int currSecond = qFloor(pts/1000000.);
            if(currSecond >= mSecondId) {
                if(currSecond > mSecondId)
                    qDebug() << QString::number(currSecond) +
                                " instead of " + QString::number(mSecondId);
                if(firstSample) {
                    firstSample = false;
                    pts = av_rescale_q(pts, audioStream->time_base, AV_TIME_BASE_Q);
                    currentSample = pts*SOUND_SAMPLERATE/1000000;
                }
                // resample frames
                float *buffer;
                av_samples_alloc((uint8_t**)&buffer, nullptr, 1,
                                 decodedFrame->nb_samples, AV_SAMPLE_FMT_FLT, 0);
                const int nSamplesT = swr_convert(
                            swrContext, (uint8_t**)&buffer,
                            decodedFrame->nb_samples,
                            (const uint8_t**)decodedFrame->data,
                            decodedFrame->nb_samples);
                // append resampled frames to data
                int firstRelSample = 0;
                int nSamplesInRange = nSamplesT;
                if(mSampleRange.fMin > currentSample) {
                    firstRelSample = mSampleRange.fMin - currentSample;
                    nSamplesInRange -= firstRelSample;
                }
                if(nSamplesInRange > 0) {
                    const int newNSamples = nSamples + nSamplesInRange;
                    const ulong newAudioDataSize = static_cast<ulong>(newNSamples) * sizeof(float);
                    void * const audioDataMem = realloc(audioData, newAudioDataSize);
                    audioData = static_cast<float*>(audioDataMem);
                    const float * const src = buffer + firstRelSample;
                    float * const dst = audioData + nSamples;
                    memcpy(dst, src, static_cast<ulong>(nSamplesInRange) * sizeof(float));
                    nSamples = newNSamples;
                }

                av_freep(&((uint8_t**)&buffer)[0]);

                currentSample += nSamplesT;
                if(currentSample >= mSampleRange.fMax) break;
                mSamples = SPtrCreate(Samples)(audioData, mSampleRange.span());
                break;
            }
            av_frame_unref(decodedFrame);
        }

        av_frame_unref(decodedFrame);
        av_packet_unref(packet);
    }

    SoundCacheHandler * const mCacheHandler;
    const AudioStreamsData * const mOpenedAudio;
    const int mSecondId;
    const SampleRange mSampleRange;
    stdsptr<Samples> mSamples;
};

class SoundCacheHandler : public HDDCachableCacheHandler {
    typedef stdsptr<SoundCacheContainer> stdptrSCC;
public:

protected:
    void loadSamples(const int& secondId) {
        const int sR = mSingleSound->getSampleRate();
        const SampleRange& range = {secondId*sR, (secondId + 1)*sR - 1};
        const auto reader = SPtrCreate(SoundReader)(this, mAudioStreamsData,
                                                    secondId, range);
        reader->scheduleTask();
    }

    const QString mFilePath;

private:
    const SingleSound * const mSingleSound;
};

#endif // SOUNDCACHEHANDLER_H
