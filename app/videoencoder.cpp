#include "videoencoder.h"
#include <QByteArray>
#include "Boxes/boundingboxrendercontainer.h"
#include "canvas.h"
VideoEncoder *VideoEncoder::mVideoEncoderInstance = nullptr;

VideoEncoder::VideoEncoder() {
    mVideoEncoderInstance = this;
}

void VideoEncoder::addContainer(
        const stdsptr<ImageCacheContainer>& cont) {
    if(!cont) return;
    cont->setBlocked(true);
    mNextContainers.append(cont);
    if(getState() != PROCESSING) scheduleTask();
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
    if(ret < 0) RuntimeThrow("Could not allocate frame data");

    return picture;
}

static void openVideo(AVCodec * const codec,
                      OutputStream * const ost) {
    AVCodecContext * const c = ost->enc;

    /* open the codec */
    if(avcodec_open2(c, codec, nullptr) < 0)
        RuntimeThrow("Could not open codec");

    /* Allocate the encoded raw picture. */
    ost->frame = allocPicture(c->pix_fmt, c->width, c->height);
    if(!ost->frame) RuntimeThrow("Could not allocate picture");

    /* copy the stream parameters to the muxer */
    int ret = avcodec_parameters_from_context(ost->st->codecpar, c);
    if(ret < 0) RuntimeThrow("Could not copy the stream parameters");
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

    ost->st = avformat_new_stream(oc, nullptr);
    if(!ost->st) RuntimeThrow("Could not alloc stream");

    AVCodecContext * const c = avcodec_alloc_context3(codec);
    if(!c) RuntimeThrow("Could not alloc an encoding context");

    ost->enc = c;

    /* Put sample parameters. */
    c->bit_rate = outSettings.videoBitrate;//settings->getVideoBitrate();
    /* Resolution must be a multiple of two. */
    c->width    = renSettings.fVideoWidth;
    c->height   = renSettings.fVideoHeight;
    /* timebase: This is the fundamental unit of time (in seconds) in terms
     * of which frame timestamps are represented. For fixed-fps content,
     * timebase should be 1/framerate and timestamp increments should be
     * identical to 1. */
    ost->st->time_base = renSettings.fTimeBase;
    c->time_base       = ost->st->time_base;

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
    if(ret < 0) RuntimeThrow("Could not make AVFrame writable");

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
    AVCodecContext *c = ost->enc;

    /* check if we want to generate more frames */
//    if(av_compare_ts(ost->next_pts, c->time_base,
//                      STREAM_DURATION, (AVRational) { 1, 1 }) >= 0)
//        return nullptr;

    if(c->pix_fmt != AV_PIX_FMT_BGRA) {
        /* as we only generate a rgba picture, we must convert it
         * to the codec pixel format if needed */
        if(!ost->sws_ctx) {
            ost->sws_ctx = sws_getContext(c->width, c->height,
                                          AV_PIX_FMT_BGRA,
                                          c->width, c->height,
                                          c->pix_fmt, SWS_BICUBIC,
                                          nullptr, nullptr, nullptr);
            if(!ost->sws_ctx)
                RuntimeThrow("Cannot initialize the conversion context");
        }
        SkPixmap pixmap;
        image->peekPixels(&pixmap);
        uint8_t * const dstSk[] = {static_cast<uint8_t*>(pixmap.writable_addr())};
        int linesizesSk[4];

        av_image_fill_linesizes(linesizesSk, AV_PIX_FMT_BGRA, image->width());
        if(av_frame_make_writable(ost->frame) < 0)
            RuntimeThrow("Could not make AVFrame writable");

        sws_scale(ost->sws_ctx, dstSk,
                  linesizesSk, 0, c->height, ost->frame->data,
                  ost->frame->linesize);
    } else {
        try {
            copyImageToFrame(ost->frame, image, c->width, c->height);
        } catch(...) {
            RuntimeThrow("Failed to copy image to frame");
        }
    }

    ost->frame->pts = ost->next_pts++;

    return ost->frame;
}

static void writeVideoFrame(AVFormatContext * const oc,
                            OutputStream * const ost,
                            const sk_sp<SkImage> &image,
                            bool * const encodeVideo) {
    AVCodecContext * const c = ost->enc;

    AVFrame * frame;
    try {
        frame = getVideoFrame(ost, image);
    } catch(...) {
        RuntimeThrow("Failed to retrieve video frame");
    }


    /* encode the image */
    const int ret = avcodec_send_frame(c, frame);
    if(ret < 0) RuntimeThrow("Error submitting a frame for encoding");

    while(ret >= 0) {
        AVPacket pkt = { 0 };

        av_init_packet(&pkt);

        const int recRet = avcodec_receive_packet(c, &pkt);
        if(recRet >= 0) {
            av_packet_rescale_ts(&pkt, c->time_base, ost->st->time_base);
            pkt.stream_index = ost->st->index;

            /* Write the compressed frame to the media file. */
            const int interRet = av_interleaved_write_frame(oc, &pkt);
            if(interRet < 0) RuntimeThrow("Error while writing video frame");
        } else if(recRet == AVERROR(EAGAIN) || recRet == AVERROR_EOF) {
            *encodeVideo = ret != AVERROR_EOF;
            break;
        } else {
            RuntimeThrow("Error encoding a video frame " + std::to_string(recRet));
        }
    }
}

static void closeStream(OutputStream * const ost) {
    if(ost) {
        if(ost->enc) avcodec_free_context(&ost->enc);
        if(ost->frame) av_frame_free(&ost->frame);
        if(ost->sws_ctx) sws_freeContext(ost->sws_ctx);
        if(ost->avr) avresample_free(&ost->avr);
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

    ost->st = avformat_new_stream(oc, nullptr);
    if(!ost->st) RuntimeThrow("Could not alloc stream");

    AVCodecContext * const c = avcodec_alloc_context3(codec);
    if(!c) RuntimeThrow("Could not alloc an encoding context");
    ost->enc = c;

    /* put sample parameters */
    c->sample_fmt     = settings.audioSampleFormat;
    c->sample_rate    = settings.audioSampleRate;
    c->channel_layout = settings.audioChannelsLayout;
    c->channels       = av_get_channel_layout_nb_channels(c->channel_layout);
    c->bit_rate       = settings.audioBitrate;

    ost->st->time_base = (AVRational) { 1, c->sample_rate };

    // some formats want stream headers to be separate
    if(oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    /* initialize sample format conversion;
     * to simplify the code, we always pass the data through lavr, even
     * if the encoder supports the generated format directly -- the price is
     * some extra data copying;
     */
    ost->avr = avresample_alloc_context();
    if(!ost->avr) RuntimeThrow("Error allocating the resampling context");

    av_opt_set_int(ost->avr, "in_sample_fmt",      AV_SAMPLE_FMT_S16,   0); // !!!
    av_opt_set_int(ost->avr, "in_sample_rate",     44100,               0);
    av_opt_set_int(ost->avr, "in_channel_layout",  AV_CH_LAYOUT_STEREO, 0);
    av_opt_set_int(ost->avr, "out_sample_fmt",     c->sample_fmt,       0);
    av_opt_set_int(ost->avr, "out_sample_rate",    c->sample_rate,      0);
    av_opt_set_int(ost->avr, "out_channel_layout", c->channel_layout,   0);

    const int ret = avresample_open(ost->avr);
    if(ret < 0) RuntimeThrow("Error opening the resampling context");
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
        if(ret < 0) RuntimeThrow("Error allocating an audio buffer");
    }

    return frame;
}

static void openAudio(AVCodec * const codec, OutputStream * const ost) {
    AVCodecContext * const c = ost->enc;

    /* open it */

    const int ret = avcodec_open2(c, codec, nullptr);
    if(ret < 0) RuntimeThrow("Could not open codec");

    /* init signal generator */
    ost->t = 0;
    const qreal tincr = 2 * M_PI * 110. / c->sample_rate;
    ost->tincr = static_cast<float>(tincr);
    /* increment frequency by 110 Hz per second */
    ost->tincr2 = static_cast<float>(tincr / c->sample_rate);

    const bool varFS = c->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE;
    const int nb_samples = varFS ? 10000 : c->frame_size;

    ost->frame = allocAudioFrame(c->sample_fmt, c->channel_layout,
                                 c->sample_rate, nb_samples);
    if(!ost->frame) RuntimeThrow("Could not alloc audio frame");

    ost->tmp_frame = allocAudioFrame(AV_SAMPLE_FMT_S16, AV_CH_LAYOUT_STEREO,
                                     44100, nb_samples); // !!!
    if(!ost->tmp_frame) RuntimeThrow("Could not alloc temporary audio frame");

    /* copy the stream parameters to the muxer */
    const int parRet = avcodec_parameters_from_context(ost->st->codecpar, c);
    if(parRet < 0) RuntimeThrow("Could not copy the stream parameters");
}

/* Prepare a 16 bit dummy audio frame of 'frame_size' samples and
 * 'nb_channels' channels. */
static AVFrame *getAudioFrame(OutputStream * const ost) {
    AVFrame * const frame = ost->tmp_frame;
    auto q = reinterpret_cast<int16_t*>(frame->data[0]);

    /* check if we want to generate more frames */
//    if(av_compare_ts(ost->next_pts, ost->enc->time_base,
//                     STREAM_DURATION, (AVRational) { 1, 1 }) >= 0)
//        return nullptr;


    for(int j = 0; j < frame->nb_samples; j++) {
        const auto v = static_cast<int16_t>(sin(ost->t) * 10000);
        for(int i = 0; i < ost->enc->channels; i++) *q++ = v;
        ost->t     += ost->tincr;
        ost->tincr += ost->tincr2;
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
    const int ret = avcodec_send_frame(ost->enc, frame);
    if(ret < 0) RuntimeThrow("Error submitting a frame for encoding");

    while(true) {
        AVPacket pkt = { 0 }; // data and size must be 0;

        av_init_packet(&pkt);

        const int recRet = avcodec_receive_packet(ost->enc, &pkt);
        if(recRet >= 0) {
            av_packet_rescale_ts(&pkt, ost->enc->time_base, ost->st->time_base);
            pkt.stream_index = ost->st->index;

            /* Write the compressed frame to the media file. */
            const int interRet = av_interleaved_write_frame(oc, &pkt);
            if(interRet < 0) RuntimeThrow("Error while writing video frame");
        } else if(recRet == AVERROR(EAGAIN) || recRet == AVERROR_EOF) {
            *encodeAudio = recRet == AVERROR(EAGAIN);
            break;
        } else {
            RuntimeThrow("Error encoding a video frame");
        }
    }
}

static void processAudioStream(AVFormatContext * const oc,
                               OutputStream * const ost,
                               bool * const audioEnabled) {
    AVFrame * const frame = getAudioFrame(ost);
    bool gotOutput = frame;

    /* feed the data to lavr */
    if(frame) {
        const int ret = avresample_convert(ost->avr, nullptr, 0, 0,
                                 frame->extended_data, frame->linesize[0],
                                 frame->nb_samples);
        if(ret < 0) RuntimeThrow("Error feeding audio data to the resampler");
    }

    while((frame && avresample_available(ost->avr) >= ost->frame->nb_samples) ||
          (!frame && avresample_get_out_samples(ost->avr, 0))) {
        /* when we pass a frame to the encoder, it may keep a reference to it
         * internally;
         * make sure we do not overwrite it here
         */
        const int makeWRet = av_frame_make_writable(ost->frame);
        if(makeWRet < 0) RuntimeThrow("Error making AVFrame writable");

        /* the difference between the two avresample calls here is that the
         * first one just reads the already converted data that is buffered in
         * the lavr output buffer, while the second one also flushes the
         * resampler */
        int sample;
        if(frame) {
            sample = avresample_read(ost->avr, ost->frame->extended_data,
                                     ost->frame->nb_samples);
        } else {
            sample = avresample_convert(ost->avr, ost->frame->extended_data,
                                        ost->frame->linesize[0], ost->frame->nb_samples,
                                        nullptr, 0, 0);
        }

        if(sample < 0) RuntimeThrow("Error while resampling");
        else if(frame && sample != ost->frame->nb_samples)
            RuntimeThrow("Too few samples returned from resampler");

        ost->frame->nb_samples = sample;

        ost->frame->pts        = ost->next_pts;
        ost->next_pts         += ost->frame->nb_samples;

        try {
            encodeAudioFrame(oc, ost, ost->frame, &gotOutput);
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

    // add streams
    mHaveVideo = 0;
    mHaveAudio = 0;
    mEncodeVideo = 0;
    mEncodeAudio = 0;
    mVideoStream = { 0 };
    mAudioStream = { 0 };
    if(mOutputSettings.videoCodec && mOutputSettings.videoEnabled) {
        try {
            addVideoStream(&mVideoStream, mFormatContext,
                             mOutputSettings, mRenderSettings);
        } catch (...) {
            RuntimeThrow("Error adding video stream");
        }
        mHaveVideo = 1;
        mEncodeVideo = 1;
    }
    if(mOutputFormat->audio_codec != AV_CODEC_ID_NONE &&
       mOutputSettings.audioEnabled) {
        try {
            addAudioStream(&mAudioStream, mFormatContext,
                             mOutputSettings);
        } catch (...) {
            RuntimeThrow("Error adding audio stream");
        }
        mHaveAudio = 1;
        mEncodeAudio = 1;
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
        if(avioRet < 0) RuntimeThrow("Could not open " + mPathByteArray.data());
    }

    const int whRet = avformat_write_header(mFormatContext, nullptr);
    if(whRet < 0) RuntimeThrow("Could not write header to " + mPathByteArray.data());
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
    qDebug() << "FINISHED";
}

void VideoEncoder::finishEncodingNow() {
    if(!mCurrentlyEncoding) return;

    /* Write the trailer, if any. The trailer must be written before you
     * close the CodecContexts open when you wrote the header; otherwise
     * av_write_trailer() may try to use memory that was freed on
     * av_codec_close(). */
    if(mEncodingSuccesfull) {
        av_write_trailer(mFormatContext);
    }

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
    clearContainers();
}

void VideoEncoder::clearContainers() {
    for(const auto &cont : _mContainers)
        cont->setBlocked(false);
    _mContainers.clear();
}

void VideoEncoder::_processUpdate() {
    bool encodeVideoT = !_mContainers.isEmpty(); // local encode
    bool encodeAudioT = true; // local encode
    while((mEncodeVideo && encodeVideoT) || (mEncodeAudio && encodeAudioT)) {
        bool avAligned = true;
        if(mEncodeAudio) {
            avAligned = av_compare_ts(mVideoStream.next_pts,
                                      mVideoStream.enc->time_base,
                                      mAudioStream.next_pts,
                                      mAudioStream.enc->time_base) <= 0;
        }
        if(mEncodeVideo && encodeVideoT && avAligned) {
            const auto cacheCont = _mContainers.at(_mCurrentContainerId);
            const auto contRage = cacheCont->getRange()*_mRenderRange;
            const int nFrames = contRage.span();
            try {
                writeVideoFrame(mFormatContext, &mVideoStream,
                                cacheCont->getImageSk(), &mEncodeVideo);
            } catch(...) {
                mUpdateException = std::current_exception();
            }
            _mCurrentContainerFrame++;
            if(_mCurrentContainerFrame >= nFrames) {
                _mCurrentContainerId++;
                _mCurrentContainerFrame = 0;
                encodeVideoT = _mCurrentContainerId < _mContainers.count();
            }
        } else if(mEncodeAudio) {
            try {
                processAudioStream(mFormatContext, &mAudioStream, &mEncodeAudio);
            } catch(...) {
                mUpdateException = std::current_exception();
            }
        }
    }
}

void VideoEncoder::beforeProcessingStarted() {
    _HDDTask::beforeProcessingStarted();
    _mCurrentContainerId = 0;
    _mCurrentContainerFrame = 0;
    _mContainers.swap(mNextContainers);
    _mRenderRange = {mRenderSettings.fMinFrame, mRenderSettings.fMaxFrame};
    if(!mCurrentlyEncoding) clearContainers();
}

void VideoEncoder::afterProcessingFinished() {
    bool firstT = true;
    for(int i = _mCurrentContainerId - 1; i >= 0; i--) {
        const auto &cont = _mContainers.at(i);
        qDebug() << "{" << cont->getRange().fMin << "," <<
                           cont->getRange().fMax << "}";
        if(firstT) {
            auto currCanvas = mRenderInstanceSettings->getTargetCanvas();
            currCanvas->setCurrentPreviewContainer(cont);
            firstT = false;
        } else {
            cont->setBlocked(false);
        }
    }
    _mContainers.clear();

    if(mInterruptEncoding) {
        mRenderInstanceSettings->setCurrentState(
                    RenderInstanceSettings::NONE);
        interrupEncoding();
        mInterruptEncoding = false;
    } else if(mUpdateException) {
        gPrintExceptionCritical(mUpdateException);
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
