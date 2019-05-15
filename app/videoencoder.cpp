#include "videoencoder.h"
#include <QByteArray>
#include "Boxes/boundingboxrendercontainer.h"
#include "canvas.h"

#define AV_RuntimeThrow(errId, message) \
{ \
    char * const errMsg = new char[AV_ERROR_MAX_STRING_SIZE]; \
    av_make_error_string(errMsg, AV_ERROR_MAX_STRING_SIZE, errId); \
    try { \
        RuntimeThrow(errMsg); \
    } catch(...) { \
        delete[] errMsg; \
        RuntimeThrow(message); \
    } \
}

VideoEncoder *VideoEncoder::mVideoEncoderInstance = nullptr;

VideoEncoder::VideoEncoder() {
    mVideoEncoderInstance = this;
}

void VideoEncoder::addContainer(
        const stdsptr<ImageCacheContainer>& cont) {
    if(!cont) return;
    cont->setBlocked(true);
    mNextContainers.append(cont);
    if(getState() < QUED || getState() > PROCESSING) scheduleTask();
}

void VideoEncoder::addContainer(const stdsptr<Samples>& cont) {
    if(!cont) return;
    mNextSoundConts.append(cont);
    if(getState() < QUED || getState() > PROCESSING) scheduleTask();
}

static AVFrame *allocPicture(enum AVPixelFormat pix_fmt,
                             const int& width, const int& height) {
    AVFrame * const picture = av_frame_alloc();
    if(!picture) RuntimeThrow("Could not allocate frame");

    picture->format = pix_fmt;
    picture->width  = width;
    picture->height = height;

    /* allocate the buffers for the frame data */
    const int ret = av_frame_get_buffer(picture, 32);
    if(ret < 0) AV_RuntimeThrow(ret, "Could not allocate frame data");

    return picture;
}

static void openVideo(AVCodec * const codec, OutputStream * const ost) {
    AVCodecContext * const c = ost->fCodec;
    ost->fNextPts = 0;
    /* open the codec */
    int ret = avcodec_open2(c, codec, nullptr);
    if(ret < 0) AV_RuntimeThrow(ret, "Could not open codec");

    /* Allocate the encoded raw picture. */
    ost->fFrame = allocPicture(c->pix_fmt, c->width, c->height);
    if(!ost->fFrame) RuntimeThrow("Could not allocate picture");

    /* copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(ost->fStream->codecpar, c);
    if(ret < 0) AV_RuntimeThrow(ret, "Could not copy the stream parameters");
}

static void addVideoStream(OutputStream * const ost,
                           AVFormatContext * const oc,
                           const OutputSettings &outSettings,
                           const RenderSettings &renSettings) {
    AVCodec * const codec = outSettings.videoCodec;

//    if(!codec) {
//        /* find the video encoder */
//        codec = avcodec_find_encoder(codec_id);
//        if(!codec) {
//            fprintf(stderr, "codec not found\n");
//            return false;
//        }
//    }

    ost->fStream = avformat_new_stream(oc, nullptr);
    if(!ost->fStream) RuntimeThrow("Could not alloc stream");

    AVCodecContext * const c = avcodec_alloc_context3(codec);
    if(!c) RuntimeThrow("Could not alloc an encoding context");

    ost->fCodec = c;

    /* Put sample parameters. */
    c->bit_rate = outSettings.videoBitrate;//settings->getVideoBitrate();
    /* Resolution must be a multiple of two. */
    c->width    = renSettings.fVideoWidth;
    c->height   = renSettings.fVideoHeight;
    /* timebase: This is the fundamental unit of time (in seconds) in terms
     * of which frame timestamps are represented. For fixed-fps content,
     * timebase should be 1/framerate and timestamp increments should be
     * identical to 1. */
    ost->fStream->time_base = renSettings.fTimeBase;
    c->time_base       = ost->fStream->time_base;

    c->gop_size      = 12; /* emit one intra frame every twelve frames at most */
    c->pix_fmt       = outSettings.videoPixelFormat;//BGRA;
    if(c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
        /* just for testing, we also add B-frames */
        c->max_b_frames = 2;
    } else if(c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
        /* Needed to avoid using macroblocks in which some coeffs overflow.
         * This does not happen with normal video, it just happens here as
         * the motion of the chroma plane does not match the luma plane. */
        c->mb_decision = 2;
    }
    /* Some formats want stream headers to be separate. */
    if(oc->oformat->flags & AVFMT_GLOBALHEADER) {
        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
}

/* Prepare a dummy image. */
static void copyImageToFrame(AVFrame * const pict,
                             const sk_sp<SkImage> &skiaImg,
                             const int width, const int height) {
    /* when we pass a frame to the encoder, it may keep a reference to it
     * internally;
     * make sure we do not overwrite it here
     */
    const int ret = av_frame_make_writable(pict);
    if(ret < 0) AV_RuntimeThrow(ret, "Could not make AVFrame writable");

    SkPixmap pixmap;
    skiaImg->peekPixels(&pixmap);

    const auto srcData = static_cast<uint8_t*>(pixmap.writable_addr());
    const auto dstData = reinterpret_cast<uint8_t*>(pict->data);
    int yi = 0;
    int xi = 0;
    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            dstData[xi++] = srcData[yi++];
            dstData[xi++] = srcData[yi++];
            dstData[xi++] = srcData[yi++];
            yi++;
            //pict->data[3][y * pict->linesize[3] + x] = imgData[yi++];
        }
    }
}

static AVFrame *getVideoFrame(OutputStream * const ost,
                              const sk_sp<SkImage> &image) {
    AVCodecContext *c = ost->fCodec;

    /* check if we want to generate more frames */
//    if(av_compare_ts(ost->next_pts, c->time_base,
//                      STREAM_DURATION, (AVRational) { 1, 1 }) >= 0)
//        return nullptr;

    if(c->pix_fmt != AV_PIX_FMT_BGRA) {
        /* as we only generate a rgba picture, we must convert it
         * to the codec pixel format if needed */
        if(!ost->fSwsCtx) {
            ost->fSwsCtx = sws_getContext(c->width, c->height,
                                          AV_PIX_FMT_BGRA,
                                          c->width, c->height,
                                          c->pix_fmt, SWS_BICUBIC,
                                          nullptr, nullptr, nullptr);
            if(!ost->fSwsCtx)
                RuntimeThrow("Cannot initialize the conversion context");
        }
        SkPixmap pixmap;
        image->peekPixels(&pixmap);
        uint8_t * const dstSk[] = {static_cast<uint8_t*>(pixmap.writable_addr())};
        int linesizesSk[4];

        av_image_fill_linesizes(linesizesSk, AV_PIX_FMT_BGRA, image->width());
        const int ret = av_frame_make_writable(ost->fFrame) ;
        if(ret < 0)
            AV_RuntimeThrow(ret, "Could not make AVFrame writable");

        sws_scale(ost->fSwsCtx, dstSk,
                  linesizesSk, 0, c->height, ost->fFrame->data,
                  ost->fFrame->linesize);
    } else {
        try {
            copyImageToFrame(ost->fFrame, image, c->width, c->height);
        } catch(...) {
            RuntimeThrow("Failed to copy image to frame");
        }
    }

    ost->fFrame->pts = ost->fNextPts++;

    return ost->fFrame;
}

static void writeVideoFrame(AVFormatContext * const oc,
                            OutputStream * const ost,
                            const sk_sp<SkImage> &image,
                            bool * const encodeVideo) {
    AVCodecContext * const c = ost->fCodec;

    AVFrame * frame;
    try {
        frame = getVideoFrame(ost, image);
    } catch(...) {
        RuntimeThrow("Failed to retrieve video frame");
    }


    // encode the image
    const int ret = avcodec_send_frame(c, frame);
    if(ret < 0) AV_RuntimeThrow(ret, "Error submitting a frame for encoding");

    while(ret >= 0) {
        AVPacket pkt;
        av_init_packet(&pkt);

        const int recRet = avcodec_receive_packet(c, &pkt);
        if(recRet >= 0) {
            av_packet_rescale_ts(&pkt, c->time_base, ost->fStream->time_base);
            pkt.stream_index = ost->fStream->index;

            // Write the compressed frame to the media file.
            const int interRet = av_interleaved_write_frame(oc, &pkt);
            if(interRet < 0) AV_RuntimeThrow(interRet, "Error while writing video frame");
        } else if(recRet == AVERROR(EAGAIN) || recRet == AVERROR_EOF) {
            *encodeVideo = ret != AVERROR_EOF;
            break;
        } else {
            AV_RuntimeThrow(recRet, "Error encoding a video frame");
        }

        av_packet_unref(&pkt);
    }
}

static void addAudioStream(OutputStream * const ost,
                           AVFormatContext * const oc,
                           const OutputSettings &settings) {
    AVCodec * const codec = settings.audioCodec;

//    /* find the audio encoder */
//    codec = avcodec_find_encoder(codec_id);
//    if(!codec) {
//        fprintf(stderr, "codec not found\n");
//        return false;
//    }

    ost->fStream = avformat_new_stream(oc, nullptr);
    if(!ost->fStream) RuntimeThrow("Could not alloc stream");

    AVCodecContext * const c = avcodec_alloc_context3(codec);
    if(!c) RuntimeThrow("Could not alloc an encoding context");
    ost->fCodec = c;

    /* put sample parameters */
    c->sample_fmt     = settings.audioSampleFormat;
    c->sample_rate    = settings.audioSampleRate;
    c->channel_layout = settings.audioChannelsLayout;
    c->channels       = av_get_channel_layout_nb_channels(c->channel_layout);
    c->bit_rate       = settings.audioBitrate;

    ost->fStream->time_base = { 1, c->sample_rate };

    // some formats want stream headers to be separate
    if(oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    /* initialize sample format conversion;
     * to simplify the code, we always pass the data through lavr, even
     * if the encoder supports the generated format directly -- the price is
     * some extra data copying;
     */
    ost->fAvr = avresample_alloc_context();
    if(!ost->fAvr) RuntimeThrow("Error allocating the resampling context");

    av_opt_set_int(ost->fAvr, "in_sample_fmt",      AV_SAMPLE_FMT_FLT,   0);
    av_opt_set_int(ost->fAvr, "in_sample_rate",     44100,               0);
    av_opt_set_int(ost->fAvr, "in_channel_layout",  AV_CH_LAYOUT_MONO, 0);
    av_opt_set_int(ost->fAvr, "out_sample_fmt",     c->sample_fmt,       0);
    av_opt_set_int(ost->fAvr, "out_sample_rate",    c->sample_rate,      0);
    av_opt_set_int(ost->fAvr, "out_channel_layout", c->channel_layout,   0);

    const int ret = avresample_open(ost->fAvr);
    if(ret < 0) AV_RuntimeThrow(ret, "Error opening the resampling context");
}

static AVFrame *allocAudioFrame(enum AVSampleFormat sample_fmt,
                                const uint64_t&  channel_layout,
                                const int& sample_rate,
                                const int& nb_samples) {
    AVFrame * const frame = av_frame_alloc();

    if(!frame) RuntimeThrow("Error allocating an audio frame");

    frame->format = sample_fmt;
    frame->channel_layout = channel_layout;
    frame->sample_rate = sample_rate;
    frame->nb_samples = nb_samples;

    if(nb_samples) {
        const int ret = av_frame_get_buffer(frame, 0);
        if(ret < 0) AV_RuntimeThrow(ret, "Error allocating an audio buffer");
    }

    return frame;
}

static void openAudio(AVCodec * const codec, OutputStream * const ost) {
    AVCodecContext * const c = ost->fCodec;

    /* open it */

    const int ret = avcodec_open2(c, codec, nullptr);
    if(ret < 0) AV_RuntimeThrow(ret, "Could not open codec");

    ost->fNextPts = 0;
    /* init signal generator */
    ost->fT = 0;
    const qreal tincr = 2 * M_PI * 110. / c->sample_rate;
    ost->fTincr = static_cast<float>(tincr);
    /* increment frequency by 110 Hz per second */
    ost->fTincr2 = static_cast<float>(tincr / c->sample_rate);

    const bool varFS = c->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE;
    const int nb_samples = varFS ? 10000 : c->frame_size;

    ost->fFrame = allocAudioFrame(c->sample_fmt, c->channel_layout,
                                  c->sample_rate, nb_samples);
    if(!ost->fFrame) RuntimeThrow("Could not alloc audio frame");

    ost->fTmpFrame = allocAudioFrame(AV_SAMPLE_FMT_FLT, AV_CH_LAYOUT_MONO,
                                     44100, nb_samples); // !!!
    if(!ost->fTmpFrame) RuntimeThrow("Could not alloc temporary audio frame");

    /* copy the stream parameters to the muxer */
    const int parRet = avcodec_parameters_from_context(ost->fStream->codecpar, c);
    if(parRet < 0) AV_RuntimeThrow(parRet, "Could not copy the stream parameters");
}

static AVFrame *getAudioFrame(OutputStream * const ost,
                              SoundIterator &iterator) {
    AVFrame * const frame = ost->fTmpFrame;
    auto q = reinterpret_cast<float*>(frame->data[0]);

    for(int j = 0; j < frame->nb_samples; j++) {
        *q++ = iterator.value();
        iterator.next();
    }

    return frame;
}

/* if a frame is provided, send it to the encoder, otherwise flush the encoder;
 * return 1 when encoding is finished, 0 otherwise
 */
static void encodeAudioFrame(AVFormatContext * const oc,
                             OutputStream * const ost,
                             AVFrame * const frame,
                             bool * const encodeAudio) {
    const int ret = avcodec_send_frame(ost->fCodec, frame);
    if(ret < 0) AV_RuntimeThrow(ret, "Error submitting a frame for encoding");

    while(true) {
        AVPacket pkt;
        av_init_packet(&pkt);

        const int recRet = avcodec_receive_packet(ost->fCodec, &pkt);
        if(recRet >= 0) {
            av_packet_rescale_ts(&pkt, ost->fCodec->time_base, ost->fStream->time_base);
            pkt.stream_index = ost->fStream->index;

            /* Write the compressed frame to the media file. */
            const int interRet = av_interleaved_write_frame(oc, &pkt);
            if(interRet < 0) AV_RuntimeThrow(interRet, "Error while writing audio frame");
        } else if(recRet == AVERROR(EAGAIN) || recRet == AVERROR_EOF) {
            *encodeAudio = recRet == AVERROR(EAGAIN);
            break;
        } else {
            AV_RuntimeThrow(recRet, "Error encoding an audio frame");
        }
    }
}

static void processAudioStream(AVFormatContext * const oc,
                               OutputStream * const ost,
                               SoundIterator &iterator,
                               bool * const audioEnabled) {
    AVFrame * const frame = getAudioFrame(ost, iterator);
    bool gotOutput = frame;

    /* feed the data to lavr */
    if(frame) {
        const int ret = avresample_convert(ost->fAvr, nullptr, 0, 0,
                                 frame->extended_data, frame->linesize[0],
                                 frame->nb_samples);
        if(ret < 0) AV_RuntimeThrow(ret, "Error feeding audio data to the resampler");
    }

    while((frame && avresample_available(ost->fAvr) >= ost->fFrame->nb_samples) ||
          (!frame && avresample_get_out_samples(ost->fAvr, 0))) {
        /* when we pass a frame to the encoder, it may keep a reference to it
         * internally;
         * make sure we do not overwrite it here
         */
        const int makeWRet = av_frame_make_writable(ost->fFrame);
        if(makeWRet < 0) AV_RuntimeThrow(makeWRet, "Error making AVFrame writable");

        /* the difference between the two avresample calls here is that the
         * first one just reads the already converted data that is buffered in
         * the lavr output buffer, while the second one also flushes the
         * resampler */
        int sample;
        if(frame) {
            sample = avresample_read(ost->fAvr, ost->fFrame->extended_data,
                                     ost->fFrame->nb_samples);
        } else {
            sample = avresample_convert(ost->fAvr, ost->fFrame->extended_data,
                                        ost->fFrame->linesize[0], ost->fFrame->nb_samples,
                                        nullptr, 0, 0);
        }

        if(sample < 0) RuntimeThrow("Error while resampling");
        else if(frame && sample != ost->fFrame->nb_samples)
            RuntimeThrow("Too few samples returned from resampler");

        ost->fFrame->nb_samples = sample;

        ost->fFrame->pts = ost->fNextPts;
        ost->fNextPts += ost->fFrame->nb_samples;

        try {
            encodeAudioFrame(oc, ost, ost->fFrame, &gotOutput);
        } catch(...) {
            RuntimeThrow("Error while encoding audio frame");
        }
    }

    *audioEnabled = gotOutput;
}

void VideoEncoder::startEncodingNow() {
    if(!mOutputFormat) {
        mOutputFormat = av_guess_format(nullptr, mPathByteArray.data(), nullptr);
        if(!mOutputFormat) {
            RuntimeThrow("No AVOutputFormat provided. "
                         "Could not guess AVOutputFormat from file extension");
        }
    }
    mFormatContext = avformat_alloc_context();
    if(!mFormatContext) RuntimeThrow("Error allocating AVFormatContext");

    mFormatContext->oformat = mOutputFormat;

    _mCurrentContainerFrame = 0;
    // add streams
    mHaveVideo = false;
    mHaveAudio = false;
    mEncodeVideo = false;
    mEncodeAudio = false;
    if(mOutputSettings.videoCodec && mOutputSettings.videoEnabled) {
        try {
            addVideoStream(&mVideoStream, mFormatContext,
                           mOutputSettings, mRenderSettings);
        } catch (...) {
            RuntimeThrow("Error adding video stream");
        }
        mHaveVideo = true;
        mEncodeVideo = true;
    }
    if(mOutputFormat->audio_codec != AV_CODEC_ID_NONE &&
       mOutputSettings.audioEnabled) {
        try {
            addAudioStream(&mAudioStream, mFormatContext, mOutputSettings);
        } catch (...) {
            RuntimeThrow("Error adding audio stream");
        }
        mHaveAudio = true;
        mEncodeAudio = true;
    }
    if(!mHaveAudio && !mHaveVideo) RuntimeThrow("No streams to render");
    // open streams
    if(mHaveVideo) {
        try {
            openVideo(mOutputSettings.videoCodec, &mVideoStream);
        } catch (...) {
            RuntimeThrow("Error opening video stream");
        }
    }
    if(mHaveAudio) {
        try {
            openAudio(mOutputSettings.audioCodec, &mAudioStream);
        } catch (...) {
            RuntimeThrow("Error opening audio stream");
        }
    }

    //av_dump_format(mFormatContext, 0, mPathByteArray.data(), 1);
    if(!(mOutputFormat->flags & AVFMT_NOFILE)) {
        const int avioRet = avio_open(&mFormatContext->pb,
                                      mPathByteArray.data(),
                                      AVIO_FLAG_WRITE);
        if(avioRet < 0) AV_RuntimeThrow(avioRet, "Could not open " + mPathByteArray.data());
    }

    const int whRet = avformat_write_header(mFormatContext, nullptr);
    if(whRet < 0) AV_RuntimeThrow(whRet, "Could not write header to " + mPathByteArray.data());
}

void VideoEncoder::startEncoding(RenderInstanceSettings * const settings) {
    if(mCurrentlyEncoding) return;
    mRenderInstanceSettings = settings;
    mRenderInstanceSettings->renderingAboutToStart();
    mOutputSettings = mRenderInstanceSettings->getOutputRenderSettings();
    mRenderSettings = mRenderInstanceSettings->getRenderSettings();
    mPathByteArray = mRenderInstanceSettings->getOutputDestination().toLatin1();
    // get format from audio file

    mOutputFormat = mOutputSettings.outputFormat;
    mSoundIterator = SoundIterator();
    try {
        startEncodingNow();
        mCurrentlyEncoding = true;
        mEncodingFinished = false;
        mRenderInstanceSettings->setCurrentState(
                    RenderInstanceSettings::RENDERING);
        mEmitter.encodingStarted();
    } catch(const std::exception& e) {
        gPrintExceptionCritical(e);
        mRenderInstanceSettings->setCurrentState(
                    RenderInstanceSettings::ERROR, e.what());
        mEmitter.encodingStartFailed();
    }
}

void VideoEncoder::interrupEncoding() {
    finishEncodingNow();
    mEmitter.encodingInterrupted();
}

void VideoEncoder::finishEncodingSuccess() {
    mRenderInstanceSettings->setCurrentState(
                RenderInstanceSettings::FINISHED);
    mEncodingSuccesfull = true;
    finishEncodingNow();
    mEmitter.encodingFinished();
}

static void flushStream(OutputStream * const ost,
                        AVFormatContext * const formatCtx) {
    if(!ost) return;
    if(!ost->fCodec) return;
    AVPacket pkt;
    av_init_packet(&pkt);
    int ret = avcodec_send_frame(ost->fCodec, nullptr);
    while(ret >= 0) {
        ret = avcodec_receive_packet(ost->fCodec, &pkt);
        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            // Write packet
            avcodec_flush_buffers(ost->fCodec);
            break;
        }
        if(pkt.pts != AV_NOPTS_VALUE)
            pkt.pts = av_rescale_q(pkt.pts, ost->fCodec->time_base,
                                   ost->fStream->time_base);
        if(pkt.dts != AV_NOPTS_VALUE)
            pkt.dts = av_rescale_q(pkt.dts, ost->fCodec->time_base,
                                   ost->fStream->time_base);
        if(pkt.duration > 0)
            pkt.duration = av_rescale_q(pkt.duration, ost->fCodec->time_base,
                                        ost->fStream->time_base);
        pkt.stream_index = ost->fStream->index;
        ret = av_interleaved_write_frame(formatCtx, &pkt);
    }
}

static void closeStream(OutputStream * const ost) {
    if(!ost) return;
    if(ost->fCodec) {
        avcodec_close(ost->fCodec);
        avcodec_free_context(&ost->fCodec);
    }
    if(ost->fFrame) av_frame_free(&ost->fFrame);
    if(ost->fTmpFrame) av_frame_free(&ost->fTmpFrame);
    if(ost->fSwsCtx) sws_freeContext(ost->fSwsCtx);
    if(ost->fAvr) avresample_free(&ost->fAvr);
    *ost = OutputStream();
}

void VideoEncoder::finishEncodingNow() {
    if(!mCurrentlyEncoding) return;

    if(mHaveVideo) flushStream(&mVideoStream, mFormatContext);
    if(mHaveAudio) flushStream(&mAudioStream, mFormatContext);

    /* Write the trailer, if any. The trailer must be written before you
     * close the CodecContexts open when you wrote the header; otherwise
     * av_write_trailer() may try to use memory that was freed on
     * av_codec_close(). */
    if(mEncodingSuccesfull) av_write_trailer(mFormatContext);

    /* Close each codec. */
    if(mHaveVideo) closeStream(&mVideoStream);
    if(mHaveAudio) closeStream(&mAudioStream);

    if(mOutputFormat) {
        if(!(mOutputFormat->flags & AVFMT_NOFILE)) {
            avio_close(mFormatContext->pb);
        }
    } else if(mFormatContext) {
        avio_close(mFormatContext->pb);
    }
    if(mFormatContext) {
        avformat_free_context(mFormatContext);
    }

    mEncodeAudio = false;
    mEncodeVideo = false;
    mCurrentlyEncoding = false;
    mEncodingSuccesfull = false;
    mNextContainers.clear();
    mNextSoundConts.clear();
    clearContainers();
}

void VideoEncoder::clearContainers() {
    for(const auto &cont : _mContainers)
        cont->setBlocked(false);
    _mContainers.clear();
    mSoundIterator.clear();
}

void VideoEncoder::processTask() {
    bool encodeVideoT = !_mContainers.isEmpty(); // local encode
    bool encodeAudioT = mSoundIterator.hasValue(); // local encode
    while((mEncodeVideo && encodeVideoT) || (mEncodeAudio && encodeAudioT)) {
        bool videoAligned = true;
        if(mEncodeAudio) {
            videoAligned = av_compare_ts(mVideoStream.fNextPts,
                                         mVideoStream.fCodec->time_base,
                                         mAudioStream.fNextPts,
                                         mAudioStream.fCodec->time_base) <= 0;
        }
        const bool encodeVideo = mEncodeVideo && encodeVideoT && videoAligned;
        if(encodeVideo) {
            const auto cacheCont = _mContainers.at(_mCurrentContainerId);
            const auto contRage = cacheCont->getRange()*_mRenderRange;
            const int nFrames = contRage.span();
            try {
                writeVideoFrame(mFormatContext, &mVideoStream,
                                cacheCont->getImageSk(), &encodeVideoT);
                avcodec_flush_buffers(mVideoStream.fCodec);
            } catch(...) {
                RuntimeThrow("Failed to write video frame");
            }
            _mCurrentContainerFrame++;
            if(_mCurrentContainerFrame >= nFrames) {
                _mCurrentContainerId++;
                _mCurrentContainerFrame = 0;
                encodeVideoT = _mCurrentContainerId < _mContainers.count();
            }
        }
        bool audioAligned = true;
        if(mEncodeVideo) {
            audioAligned = av_compare_ts(mVideoStream.fNextPts,
                                         mVideoStream.fCodec->time_base,
                                         mAudioStream.fNextPts,
                                         mAudioStream.fCodec->time_base) >= 0;
        }
        const bool encodeAudio = mEncodeAudio && encodeAudioT && audioAligned;
        if(encodeAudio) {
            try {
                processAudioStream(mFormatContext, &mAudioStream,
                                   mSoundIterator, &encodeAudioT);
                avcodec_flush_buffers(mAudioStream.fCodec);
            } catch(...) {
                RuntimeThrow("Failed to process audio stream");
            }
            encodeAudioT = mSoundIterator.hasValue();
        }
        if(!encodeVideo && !encodeAudio) break;
    }
}


void VideoEncoder::beforeProcessing() {
    _mCurrentContainerId = 0;
    _mContainers.swap(mNextContainers);
    for(const auto& sound : mNextSoundConts)
        mSoundIterator.add(sound);
    mNextSoundConts.clear();
    _mRenderRange = {mRenderSettings.fMinFrame, mRenderSettings.fMaxFrame};
    if(!mCurrentlyEncoding) clearContainers();
}

void VideoEncoder::afterProcessing() {
    for(int i = 0; i < _mCurrentContainerId; i++) {
        const auto &cont = _mContainers.at(i);
        if(i == _mCurrentContainerId - 1) {
            auto currCanvas = mRenderInstanceSettings->getTargetCanvas();
            currCanvas->setCurrentPreviewContainer(cont);
        } else {
            cont->setBlocked(false);
        }
    }
    for(int i = _mContainers.count() - 1; i >= _mCurrentContainerId; i--) {
        const auto &cont = _mContainers.at(i);
        mNextContainers.prepend(cont);
    }
    _mContainers.clear();

    if(mInterruptEncoding) {
        mRenderInstanceSettings->setCurrentState(
                    RenderInstanceSettings::NONE);
        interrupEncoding();
        mInterruptEncoding = false;
    } else if(unhandledException()) {
        gPrintExceptionCritical(handleException());
        mRenderInstanceSettings->setCurrentState(
                    RenderInstanceSettings::ERROR, "Error");
        finishEncodingNow();
        mEmitter.encodingFailed();
    } else if(mEncodingFinished) finishEncodingSuccess();
    else if(!mNextContainers.isEmpty()) scheduleTask();
}

VideoEncoderEmitter *VideoEncoder::getVideoEncoderEmitter() {
    return mVideoEncoderInstance->getEmitter();
}

void VideoEncoder::sFinishEncoding() {
    mVideoEncoderInstance->finishCurrentEncoding();
}

bool VideoEncoder::sEncodingSuccessfulyStarted() {
    return mVideoEncoderInstance->getCurrentlyEncoding();
}

void VideoEncoder::sInterruptEncoding() {
    mVideoEncoderInstance->interruptCurrentEncoding();
}

void VideoEncoder::sStartEncoding(RenderInstanceSettings *settings) {
    mVideoEncoderInstance->startNewEncoding(settings);
}

void VideoEncoder::sAddCacheContainerToEncoder(
        const stdsptr<ImageCacheContainer> &cont) {
    mVideoEncoderInstance->addContainer(cont);
}

void VideoEncoder::sAddCacheContainerToEncoder(
        const stdsptr<Samples> &cont) {
    mVideoEncoderInstance->addContainer(cont);
}
