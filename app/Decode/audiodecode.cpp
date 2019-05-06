#include "audiodecode.h"
#include "Sound/soundcomposition.h"
extern "C" {
    #include <libavutil/opt.h>
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswresample/swresample.h>
    #include <libswscale/swscale.h>
}

int gDecodeSoundDataRange(const char* path,
                          const SampleRange &range,
                          float *&audioData) {
    // get format from audio file
    AVFormatContext* format = avformat_alloc_context();
    if(avformat_open_input(&format, path, nullptr, nullptr) != 0) {
        fprintf(stderr, "Could not open file '%s'\n", path);
        return -1;
    }
    if(avformat_find_stream_info(format, nullptr) < 0) {
        fprintf(stderr, "Could not retrieve stream info from file '%s'\n", path);
        return -1;
    }

    // Find the index of the first audio stream
    int audioStreamIndex = -1;
    for(uint i = 0; i < format->nb_streams; i++) {
        AVStream *streamT = format->streams[i];
        const AVMediaType &mediaType = streamT->codecpar->codec_type;
        if(mediaType == AVMEDIA_TYPE_AUDIO) {
            audioStreamIndex = static_cast<int>(i);
            break;
        }
    }
    if(audioStreamIndex == -1) {
        fprintf(stderr,
                "Could not retrieve audio stream from file '%s'\n", path);
        return -1;
    }
    AVCodecContext *audioCodec = nullptr;
    struct SwrContext *swr = nullptr;

    AVStream* audioStream = format->streams[audioStreamIndex];
    // find & open codec
    audioCodec = audioStream->codec;
    if(avcodec_open2(audioCodec, avcodec_find_decoder(audioCodec->codec_id), nullptr) < 0) {
        fprintf(stderr, "Failed to open decoder for stream #%u in file '%s'\n",
                audioStreamIndex, path);
        return -1;
    }

    // prepare resampler
    swr = swr_alloc();
    av_opt_set_int(swr, "in_channel_count",  audioCodec->channels, 0);
    av_opt_set_int(swr, "out_channel_count", 1, 0);
    av_opt_set_int(swr, "in_channel_layout",  audioCodec->channel_layout, 0);
    av_opt_set_int(swr, "out_channel_layout", AV_CH_LAYOUT_MONO, 0);
    av_opt_set_int(swr, "in_sample_rate", audioCodec->sample_rate, 0);
    av_opt_set_int(swr, "out_sample_rate", SOUND_SAMPLERATE, 0);
    av_opt_set_sample_fmt(swr, "in_sample_fmt",  audioCodec->sample_fmt, 0);
    av_opt_set_sample_fmt(swr, "out_sample_fmt", AV_SAMPLE_FMT_FLT,  0);
    swr_init(swr);
    if(!swr_is_initialized(swr)) {
        fprintf(stderr, "Resampler has not been properly initialized\n");
        return -1;
    }

    // prepare to read data
    AVPacket packet;
    av_init_packet(&packet);
    AVFrame* audioFrame = av_frame_alloc();
    if(!audioFrame) {
        fprintf(stderr, "Error allocating the frame\n");
        return -1;
    }

    // iterate through frames
    audioData = nullptr;
    int nSamples = 0;
    int minFrameTsms = range.fMin * 1000 / SOUND_SAMPLERATE - 1;

    const int64_t seekFrame = av_rescale(minFrameTsms,
                                   audioStream->time_base.den,
                                   audioStream->time_base.num)/1000;

    if(range.fMin != 0) {
        if(avformat_seek_file(format, audioStreamIndex, 0,
                              seekFrame, seekFrame,
                AVSEEK_FLAG_FRAME) < 0) {
            fprintf(stderr, "Error seeking the audio frame\n");
            return -1;// 0;
        }
    }
    bool firstFrame = true;
    int currentSample = 0;
    while(av_read_frame(format, &packet) >= 0) {
        if(packet.stream_index == audioStreamIndex) {
            // decode one frame
            int gotFrame;
            if(avcodec_decode_audio4(audioCodec, audioFrame, &gotFrame, &packet) < 0) {
                break;
            }
            if(!gotFrame) continue;
            if(firstFrame) {
                firstFrame = false;
                int64_t pts = av_frame_get_best_effort_timestamp(audioFrame);
                pts = av_rescale_q(pts, audioStream->time_base, AV_TIME_BASE_Q);
                currentSample = pts*SOUND_SAMPLERATE/1000000;
            }
            // resample frames
            float *buffer;
            av_samples_alloc((uint8_t**)&buffer, nullptr, 1,
                             audioFrame->nb_samples, AV_SAMPLE_FMT_FLT, 0);
            const int nSamplesT = swr_convert(
                        swr, (uint8_t**)&buffer,
                        audioFrame->nb_samples,
                        (const uint8_t**)audioFrame->data,
                        audioFrame->nb_samples);
            // append resampled frames to data
            int firstRelSample = 0;
            int nSamplesInRange = nSamplesT;
            if(range.fMin > currentSample) {
                firstRelSample = range.fMin - currentSample;
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
            if(currentSample >= range.fMax) break;
        }

        av_free_packet(&packet);
    }

    // clean up
    av_frame_free(&audioFrame);
    if(swr) swr_free(&swr);
    if(audioCodec) avcodec_close(audioCodec);
    avformat_free_context(format);

    // success
    return 0;
}


int gDecodeAudioFile(const char* path,
                     const int sample_rate,
                     float** audioData,
                     int* size) {
    // get format from audio file
    AVFormatContext* format = avformat_alloc_context();
    if(avformat_open_input(&format, path, nullptr, nullptr) != 0) {
        fprintf(stderr, "Could not open file '%s'\n", path);
        return -1;
    }
    if(avformat_find_stream_info(format, nullptr) < 0) {
        fprintf(stderr, "Could not retrieve stream info from file '%s'\n", path);
        return -1;
    }

    // Find the index of the first audio stream
    int audioStreamIndex = -1;
    for(uint i = 0; i < format->nb_streams; i++) {
        AVStream *streamT = format->streams[i];
        const AVMediaType &mediaType = streamT->codecpar->codec_type;
        if(mediaType == AVMEDIA_TYPE_AUDIO) {
            audioStreamIndex = i;
            break;
        }
    }
    if(audioStreamIndex == -1) {
        fprintf(stderr,
                "Could not retrieve audio stream from file '%s'\n", path);
        return -1;
    }
    AVCodecContext *audioCodec = nullptr;
    struct SwrContext *swr = nullptr;

    AVStream* audioStream = format->streams[audioStreamIndex];
    // find & open codec
    audioCodec = audioStream->codec;
    if(avcodec_open2(audioCodec, avcodec_find_decoder(audioCodec->codec_id), nullptr) < 0) {
        fprintf(stderr, "Failed to open decoder for stream #%u in file '%s'\n",
                audioStreamIndex, path);
        return -1;
    }

    // prepare resampler
    swr = swr_alloc();
    av_opt_set_int(swr, "in_channel_count",  audioCodec->channels, 0);
    av_opt_set_int(swr, "out_channel_count", 1, 0);
    av_opt_set_int(swr, "in_channel_layout",  audioCodec->channel_layout, 0);
    av_opt_set_int(swr, "out_channel_layout", AV_CH_LAYOUT_MONO, 0);
    av_opt_set_int(swr, "in_sample_rate", audioCodec->sample_rate, 0);
    av_opt_set_int(swr, "out_sample_rate", sample_rate, 0);
    av_opt_set_sample_fmt(swr, "in_sample_fmt",  audioCodec->sample_fmt, 0);
    av_opt_set_sample_fmt(swr, "out_sample_fmt", AV_SAMPLE_FMT_FLT,  0);
    swr_init(swr);
    if(!swr_is_initialized(swr)) {
        fprintf(stderr, "Resampler has not been properly initialized\n");
        return -1;
    }

    // prepare to read data
    AVPacket packet;
    av_init_packet(&packet);
    AVFrame* frame = av_frame_alloc();
    if(!frame) {
        fprintf(stderr, "Error allocating the frame\n");
        return -1;
    }

    // iterate through frames
    *audioData = nullptr;
    *size = 0;
    while(av_read_frame(format, &packet) >= 0) {
        if(packet.stream_index == audioStreamIndex) {
            // decode one frame
            int gotFrame;
            if(avcodec_decode_audio4(audioCodec, frame, &gotFrame, &packet) < 0) {
                break;
            }
            if(!gotFrame) continue;
            // resample frames
            float *buffer;
            av_samples_alloc((uint8_t**) &buffer, nullptr, 1,
                             frame->nb_samples, AV_SAMPLE_FMT_FLT, 0);
            int frame_count = swr_convert(swr,
                                          (uint8_t**) &buffer,
                                          frame->nb_samples,
                                          (const uint8_t**) frame->data,
                                          frame->nb_samples);
            // append resampled frames to data
            *audioData = (float*) realloc(*audioData,
                                     (*size + frame->nb_samples) * sizeof(float));
            memcpy(*audioData + *size, buffer, frame_count * sizeof(float));

            av_freep(&((uint8_t**) &buffer)[0]);
            *size += frame_count;
        }
        av_free_packet(&packet);
    }

    // clean up
    av_frame_free(&frame);
    if(swr) swr_free(&swr);
    if(audioCodec) avcodec_close(audioCodec);
    avformat_free_context(format);

    // success
    return 0;
}
