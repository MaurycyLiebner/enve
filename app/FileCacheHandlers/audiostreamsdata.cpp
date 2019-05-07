#include "audiostreamsdata.h"
#include "Sound/soundcomposition.h"

stdsptr<const AudioStreamsData> AudioStreamsData::sOpen(
        const QString &path, AVFormatContext * const formatContext) {
    const auto result = std::shared_ptr<AudioStreamsData>(
                new AudioStreamsData, AudioStreamsData::sDestroy);
    result->open(path, formatContext);
    return result;
}

stdsptr<const AudioStreamsData> AudioStreamsData::sOpen(const QString &path) {
    const auto result = std::shared_ptr<AudioStreamsData>(
                new AudioStreamsData, AudioStreamsData::sDestroy);
    result->open(path);
    return result;
}

void AudioStreamsData::open(const QString &path,
                            AVFormatContext * const formatContext) {
    try {
        fPath = path;
        open(formatContext);
    } catch(...) {
        fPath.clear();
        RuntimeThrow("Failed to set audio file path to '" + path.toStdString() + "'.");
    }
}

void AudioStreamsData::open(const QString &path) {
    try {
        fPath = path;
        open();
    } catch(...) {
        fPath.clear();
        RuntimeThrow("Failed to set audio file path to '" + path.toStdString() + "'.");
    }
}

void AudioStreamsData::close() {
    fOpened = false;

    if(fFormatContext) avformat_close_input(&fFormatContext);
    if(fPacket) av_packet_free(&fPacket);
    if(fSwrContext) swr_free(&fSwrContext);
    if(fCodecContext) avcodec_close(fCodecContext);
    if(fDecodedFrame) av_frame_free(&fDecodedFrame);
    if(fFormatContext) avformat_free_context(fFormatContext);

    fFormatContext = nullptr;
    fAudioStreamIndex = -1;
    fAudioStream = nullptr;
    fPacket = nullptr;
    fDecodedFrame = nullptr;
    fCodecContext = nullptr;
    fSwrContext = nullptr;
}

void AudioStreamsData::open() {
    if(fOpened) return;
    const auto stdString = fPath.toStdString();
    const char * const path = stdString.c_str();
    try {
        open(path);
    } catch(...) {
        close();
        RuntimeThrow("Failed to setup audio stream for '" + path + "'.");
    }
}

void AudioStreamsData::open(const char * const path) {
    auto formatContext = avformat_alloc_context();
    if(!formatContext) RuntimeThrow("Error allocating AVFormatContext");
    if(avformat_open_input(&formatContext, path, nullptr, nullptr) != 0) {
        RuntimeThrow("Could not open file");
    }
    if(avformat_find_stream_info(formatContext, nullptr) < 0) {
        RuntimeThrow("Could not retrieve stream info");
    }
    open(formatContext);
}

void AudioStreamsData::open(AVFormatContext * const formatContext) {
    fFormatContext = formatContext;
    // Find the index of the first audio stream
    fAudioStreamIndex = -1;
    const AVCodecParameters *audCodecPars = nullptr;
    const AVCodec *audCodec = nullptr;
    fAudioStream = nullptr;
    for(uint i = 0; i < fFormatContext->nb_streams; i++) {
        AVStream * const  iStream = fFormatContext->streams[i];
        const AVCodecParameters * const iCodecPars = iStream->codecpar;
        const AVMediaType &iMediaType = iCodecPars->codec_type;
        if(iMediaType == AVMEDIA_TYPE_AUDIO) {
            fAudioStreamIndex = static_cast<int>(i);
            audCodecPars = iCodecPars;
            audCodec = avcodec_find_decoder(audCodecPars->codec_id);
            fAudioStream = iStream;
            fDurationSec = fAudioStream->duration*
                    fAudioStream->time_base.num/fAudioStream->time_base.den;
            qDebug() << "duration " << fDurationSec;
            break;
        }
    }
    if(fAudioStreamIndex == -1)
        RuntimeThrow("Could not retrieve audio stream");
    if(!audCodec) RuntimeThrow("Unsuported codec");

    fCodecContext = avcodec_alloc_context3(audCodec);
    if(!fCodecContext) RuntimeThrow("Error allocating AVCodecContext");
    if(avcodec_parameters_to_context(fCodecContext, audCodecPars) < 0) {
        RuntimeThrow("Failed to copy codec params to codec context");
    }

    if(avcodec_open2(fCodecContext, audCodec, nullptr) < 0) {
        RuntimeThrow("Failed to open codec");
    }
    const auto sampleFormat = static_cast<AVSampleFormat>(audCodecPars->format);

    fSwrContext = swr_alloc();
    av_opt_set_int(fSwrContext, "in_channel_count",  audCodecPars->channels, 0);
    av_opt_set_int(fSwrContext, "out_channel_count", 1, 0);
    av_opt_set_int(fSwrContext, "in_channel_layout",  audCodecPars->channel_layout, 0);
    av_opt_set_int(fSwrContext, "out_channel_layout", AV_CH_LAYOUT_MONO, 0);
    av_opt_set_int(fSwrContext, "in_sample_rate", audCodecPars->sample_rate, 0);
    av_opt_set_int(fSwrContext, "out_sample_rate", SOUND_SAMPLERATE, 0);
    av_opt_set_sample_fmt(fSwrContext, "in_sample_fmt", sampleFormat, 0);
    av_opt_set_sample_fmt(fSwrContext, "out_sample_fmt", AV_SAMPLE_FMT_FLT,  0);
    swr_init(fSwrContext);
    if(!swr_is_initialized(fSwrContext)) {
        RuntimeThrow("Resampler has not been properly initialized");
    }

    fPacket = av_packet_alloc();
    if(!fPacket) RuntimeThrow("Error allocating AVPacket");
    fDecodedFrame = av_frame_alloc();
    if(!fDecodedFrame) RuntimeThrow("Error allocating AVFrame");

    fOpened = true;
}
