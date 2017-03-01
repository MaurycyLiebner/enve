#include "singlesound.h"
#include "soundcomposition.h"

extern "C" {
    #include <libavutil/opt.h>
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswresample/swresample.h>
    #include <libswscale/swscale.h>
}

int decode_audio_file(const char* path,
                      const int sample_rate,
                      float** audioData,
                      int* size) {

    // initialize all muxers, demuxers and protocols for libavformat
    // (does nothing if called twice during the course of one program execution)
    av_register_all();

    // get format from audio file
    AVFormatContext* format = avformat_alloc_context();
    if (avformat_open_input(&format, path, NULL, NULL) != 0) {
        fprintf(stderr, "Could not open file '%s'\n", path);
        return -1;
    }
    if (avformat_find_stream_info(format, NULL) < 0) {
        fprintf(stderr, "Could not retrieve stream info from file '%s'\n", path);
        return -1;
    }

    // Find the index of the first audio stream
    int audioStreamIndex = -1;
    for (uint i = 0; i< format->nb_streams; i++) {
        const AVMediaType &mediaType = format->streams[i]->codec->codec_type;
        if(mediaType == AVMEDIA_TYPE_AUDIO) {
            audioStreamIndex = i;
            break;
        }
    }
    if (audioStreamIndex == -1) {
        fprintf(stderr,
                "Could not retrieve audio stream from file '%s'\n", path);
        return -1;
    }
    AVCodecContext *audioCodec = NULL;
    struct SwrContext *swr = NULL;

    AVStream* audioStream = format->streams[audioStreamIndex];
    // find & open codec
    audioCodec = audioStream->codec;
    if (avcodec_open2(audioCodec, avcodec_find_decoder(audioCodec->codec_id), NULL) < 0) {
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
    if (!swr_is_initialized(swr)) {
        fprintf(stderr, "Resampler has not been properly initialized\n");
        return -1;
    }

    // prepare to read data
    AVPacket packet;
    av_init_packet(&packet);
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Error allocating the frame\n");
        return -1;
    }

    // iterate through frames
    *audioData = NULL;
    *size = 0;
    while(av_read_frame(format, &packet) >= 0) {
        if(packet.stream_index == audioStreamIndex) {
            // decode one frame
            int gotFrame;
            if (avcodec_decode_audio4(audioCodec, frame, &gotFrame, &packet) < 0) {
                break;
            }
            if (!gotFrame) {
                continue;
            }
            // resample frames
            double* buffer;
            av_samples_alloc((uint8_t**) &buffer, NULL, 1,
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
            *size += frame_count;
        }
        av_free_packet(&packet);
    }

    // clean up
    av_frame_free(&frame);
    if(swr != NULL) {
        swr_free(&swr);
    }
    if(audioCodec != NULL) {
        avcodec_close(audioCodec);
    }
    avformat_free_context(format);

    // success
    return 0;
}

SingleSound::SingleSound() :
    ComplexAnimator() {
    setName("Sound");

    mVolumeAnimator.incNumberPointers();
    addChildAnimator(&mVolumeAnimator);
    mVolumeAnimator.setValueRange(0, 200);
    mVolumeAnimator.setCurrentValue(100);
    mVolumeAnimator.setName("Volume");
}

SingleSound::SingleSound(const QString &path) :
    SingleSound() {
    setFilePath(path);
}

void SingleSound::setFilePath(const QString &path) {
    mPath = path;
    reloadDataFromFile();
}

void SingleSound::reloadDataFromFile() {
    if(mSrcData != NULL) {
        delete[] mSrcData;
        mSrcData = NULL;
        mSrcSampleCount = 0;
    }
    if(!mPath.isEmpty()) {
        if(QFile(mPath).exists()) {
            decode_audio_file(mPath.toLatin1().data(), SAMPLERATE,
                              &mSrcData, &mSrcSampleCount);
        }
    }
    prepareFinalData();
}

int SingleSound::getStartFrame() const {
    return mStartFrame;
}

int SingleSound::getSampleCount() const {
    return mFinalSampleCount;
}

void SingleSound::prepareFinalData() {
    if(mSrcData == NULL) {
        mFinalData = NULL;
        mFinalSampleCount = 0;
    } else {
        memcpy(mFinalData, mSrcData, mSrcSampleCount*sizeof(float));
        if(mVolumeAnimator.hasKeys() ||
           qAbs(mVolumeAnimator.getCurrentValue() - 100.) > 0.1) {
            for(int i = 0; i < mSrcSampleCount; i++) {
                mFinalData[i] = mFinalData[i]*
                        mVolumeAnimator.getCurrentValue()/100.;
            }
        }
        mFinalSampleCount = mSrcSampleCount;
    }
}

const float *SingleSound::getFinalData() const {
    return mFinalData;
}
