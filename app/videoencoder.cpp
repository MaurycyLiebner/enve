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
    scheduleTask();
}

static AVFrame *alloc_picture(enum AVPixelFormat pix_fmt,
                              int width, int height) {
    AVFrame *picture;
    int ret;

    picture = av_frame_alloc();
    if(!picture) return nullptr;

    picture->format = pix_fmt;
    picture->width  = width;
    picture->height = height;

    /* allocate the buffers for the frame data */
    ret = av_frame_get_buffer(picture, 32);
    if(ret < 0) {
        fprintf(stderr, "Could not allocate frame data.\n");
        return nullptr;
    }

    return picture;
}

static bool open_video(AVCodec *codec,
                       OutputStream *ost,
                       QString &error) {
    AVCodecContext *c;
    int ret;

    c = ost->enc;

    /* open the codec */
    if(avcodec_open2(c, codec, nullptr) < 0) {
       error = "Could not open codec";
       return false;
    }

    /* Allocate the encoded raw picture. */
    ost->frame = alloc_picture(c->pix_fmt, c->width, c->height);
    if(!ost->frame) {
        error =  "Could not allocate picture";
        return false;
    }

    /* copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(ost->st->codecpar, c);
    if(ret < 0) {
        error = "Could not copy the stream parameters";
        return false;
    }
    return true;
}

static bool add_video_stream(OutputStream *ost,
                             AVFormatContext *oc,
                             const OutputSettings &outSettings,
                             const RenderSettings &renSettings,
                             QString &error) {
    AVCodecContext *c;
    AVCodec *codec = outSettings.videoCodec;

//    if(!codec) {
//        /* find the video encoder */
//        codec = avcodec_find_encoder(codec_id);
//        if(!codec) {
//            fprintf(stderr, "codec not found\n");
//            return false;
//        }
//    }

    ost->st = avformat_new_stream(oc, nullptr);
    if(!ost->st) {
        error = "Could not alloc stream";
        return false;
    }

    c = avcodec_alloc_context3(codec);
    if(!c) {
        error = "Could not alloc an encoding context";
        return false;
    }
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
    return true;
}

/* Prepare a dummy image. */
static bool copyImageToFrame(AVFrame *pict,
                            const sk_sp<SkImage> &skiaImg,
                            int width, int height,
                             QString &error) {
    int ret;

    /* when we pass a frame to the encoder, it may keep a reference to it
     * internally;
     * make sure we do not overwrite it here
     */
    ret = av_frame_make_writable(pict);
    if(ret < 0) {
        error = "Could not make AVFrame writable";
        return false;
    }

    SkPixmap pixmap;
    skiaImg->peekPixels(&pixmap);

    unsigned char *imgData = (unsigned char*)pixmap.writable_addr();
    unsigned char *dstData = (unsigned char*)pict->data;
    int yi = 0;
    int xi = 0;
    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            dstData[xi++] = imgData[yi++];
            dstData[xi++] = imgData[yi++];
            dstData[xi++] = imgData[yi++];
            yi++;
            //pict->data[3][y * pict->linesize[3] + x] = imgData[yi++];
        }
    }
    return true;
}

static AVFrame *get_video_frame(OutputStream *ost,
                                const sk_sp<SkImage> &image,
                                QString &error) {
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
                                          c->pix_fmt,
                                          SWS_BICUBIC, nullptr, nullptr, nullptr);
            if(!ost->sws_ctx) {
                error = "Cannot initialize the conversion context";
                return nullptr;
            }
        }
        SkPixmap pixmap;
        image->peekPixels(&pixmap);
        uint8_t *dstSk[] = {(unsigned char*)pixmap.writable_addr()};
        int linesizesSk[4];

        av_image_fill_linesizes(linesizesSk,
                                AV_PIX_FMT_BGRA,
                                image->width());
        if(av_frame_make_writable(ost->frame) < 0) {
            error = "Could not make AVFrame writable";
            return nullptr;
        }

        sws_scale(ost->sws_ctx, dstSk,
                  linesizesSk, 0, c->height, ost->frame->data,
                  ost->frame->linesize);
    } else {
        if(!copyImageToFrame(ost->frame, image, c->width, c->height, error)) {
            return nullptr;
        }
    }

    ost->frame->pts = ost->next_pts++;

    return ost->frame;
}

static bool write_video_frame(AVFormatContext *oc,
                              OutputStream *ost,
                              const sk_sp<SkImage> &image,
                              int *encodeVideo,
                              QString &error) {
    int ret;
    AVCodecContext *c;

    c = ost->enc;

    AVFrame *frame = get_video_frame(ost, image, error);
    if(!frame) return false;

    /* encode the image */
    ret = avcodec_send_frame(c, frame);
    if(ret < 0) {
        error = "Error submitting a frame for encoding";
        return false;
    }

    while(ret >= 0) {
        AVPacket pkt = { 0 };

        av_init_packet(&pkt);

        ret = avcodec_receive_packet(c, &pkt);
        if(ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
            error = "Error encoding a video frame";
            return false;
        } else if(ret >= 0) {
            av_packet_rescale_ts(&pkt, c->time_base, ost->st->time_base);
            pkt.stream_index = ost->st->index;

            /* Write the compressed frame to the media file. */
            ret = av_interleaved_write_frame(oc, &pkt);
            if(ret < 0) {
                error = "Error while writing video frame";
                return false;
            }
        }
    }

    *encodeVideo = ret != AVERROR_EOF;
    return true;
}

static void close_stream(AVFormatContext *oc, OutputStream *ost) {
    if(ost) {
        if(ost->enc) {
            avcodec_free_context(&ost->enc);
        }
        if(ost->frame) {
            av_frame_free(&ost->frame);
        }
        if(ost->sws_ctx) {
            sws_freeContext(ost->sws_ctx);
        }
        if(ost->avr) {
            avresample_free(&ost->avr);
        }
    }
}

static bool add_audio_stream(OutputStream *ost,
                             AVFormatContext *oc,
                             const OutputSettings &settings,
                             QString &error) {
    AVCodecContext *c;
    AVCodec *codec = settings.audioCodec;
    int ret;

//    /* find the audio encoder */
//    codec = avcodec_find_encoder(codec_id);
//    if(!codec) {
//        fprintf(stderr, "codec not found\n");
//        return false;
//    }

    ost->st = avformat_new_stream(oc, nullptr);
    if(!ost->st) {
        error = "Could not alloc stream";
        return false;
    }

    c = avcodec_alloc_context3(codec);
    if(!c) {
        error = "Could not alloc an encoding context";
        return false;
    }
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
    if(!ost->avr) {
        error = "Error allocating the resampling context";
        return false;
    }

    av_opt_set_int(ost->avr, "in_sample_fmt",      AV_SAMPLE_FMT_S16,   0); // !!!
    av_opt_set_int(ost->avr, "in_sample_rate",     44100,               0);
    av_opt_set_int(ost->avr, "in_channel_layout",  AV_CH_LAYOUT_STEREO, 0);
    av_opt_set_int(ost->avr, "out_sample_fmt",     c->sample_fmt,       0);
    av_opt_set_int(ost->avr, "out_sample_rate",    c->sample_rate,      0);
    av_opt_set_int(ost->avr, "out_channel_layout", c->channel_layout,   0);

    ret = avresample_open(ost->avr);
    if(ret < 0) {
        error = "Error opening the resampling context";
        return false;
    }
    return true;
}

static AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt,
                                  uint64_t channel_layout,
                                  int sample_rate, int nb_samples,
                                  QString &error) {
    AVFrame *frame = av_frame_alloc();
    int ret;

    if(!frame) {
        error = "Error allocating an audio frame";
        return nullptr;
    }

    frame->format = sample_fmt;
    frame->channel_layout = channel_layout;
    frame->sample_rate = sample_rate;
    frame->nb_samples = nb_samples;

    if(nb_samples) {
        ret = av_frame_get_buffer(frame, 0);
        if(ret < 0) {
            error = "Error allocating an audio buffer";
            return nullptr;
        }
    }

    return frame;
}

static bool open_audio(AVCodec *codec, OutputStream *ost,
                       QString &error) {
    AVCodecContext *c;
    int nb_samples, ret;

    c = ost->enc;

    /* open it */
    if(avcodec_open2(c, codec, nullptr) < 0) {
        error = "Could not open codec";
        return false;
    }

    /* init signal generator */
    ost->t = 0;
    ost->tincr = 2 * M_PI * 110.0 / c->sample_rate;
    /* increment frequency by 110 Hz per second */
    ost->tincr2 = 2 * M_PI * 110.0 / c->sample_rate / c->sample_rate;

    if(c->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE) {
        nb_samples = 10000;
    } else {
        nb_samples = c->frame_size;
    }

    ost->frame = alloc_audio_frame(c->sample_fmt, c->channel_layout,
                                   c->sample_rate, nb_samples,
                                   error);
    if(ost->frame == nullptr) {
        error = "Could not alloc audio frame";
        return false;
    }
    ost->tmp_frame = alloc_audio_frame(AV_SAMPLE_FMT_S16, AV_CH_LAYOUT_STEREO,
                                       44100, nb_samples, error); // !!!
    if(ost->tmp_frame == nullptr) {
        error = "Could not alloc temporary audio frame";
        return false;
    }

    /* copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(ost->st->codecpar, c);
    if(ret < 0) {
        error = "Could not copy the stream parameters";
        return false;
    }
    return true;
}

/* Prepare a 16 bit dummy audio frame of 'frame_size' samples and
 * 'nb_channels' channels. */
static AVFrame *get_audio_frame(OutputStream *ost) {
    AVFrame *frame = ost->tmp_frame;
    int j, i, v;
    int16_t *q = (int16_t*)frame->data[0];

    /* check if we want to generate more frames */
//    if(av_compare_ts(ost->next_pts, ost->enc->time_base,
//                     STREAM_DURATION, (AVRational) { 1, 1 }) >= 0)
//        return nullptr;


    for(j = 0; j < frame->nb_samples; j++) {
        v = (int)(sin(ost->t) * 10000);
        for(i = 0; i < ost->enc->channels; i++) {
            *q++ = v;
        }
        ost->t     += ost->tincr;
        ost->tincr += ost->tincr2;
    }

    return frame;
}


/* if a frame is provided, send it to the encoder, otherwise flush the encoder;
 * return 1 when encoding is finished, 0 otherwise
 */
static bool encode_audio_frame(AVFormatContext *oc,
                               OutputStream *ost,
                               AVFrame *frame,
                               int *encodeAudio,
                               QString &error) {
    int ret;

    ret = avcodec_send_frame(ost->enc, frame);
    if(ret < 0) {
        error = "Error submitting a frame for encoding";
        return false;
    }

    while(ret >= 0) {
        AVPacket pkt = { 0 }; // data and size must be 0;

        av_init_packet(&pkt);

        ret = avcodec_receive_packet(ost->enc, &pkt);
        if(ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
            error = "Error encoding a video frame";
            return false;
        } else if(ret >= 0) {
            av_packet_rescale_ts(&pkt, ost->enc->time_base, ost->st->time_base);
            pkt.stream_index = ost->st->index;

            /* Write the compressed frame to the media file. */
            ret = av_interleaved_write_frame(oc, &pkt);
            if(ret < 0) {
                error = "Error while writing video frame";
                return false;
            }
        }
    }

    *encodeAudio |= ret == AVERROR_EOF;
    return true;
}

static bool process_audio_stream(AVFormatContext *oc,
                                 OutputStream *ost,
                                 int *audioEnabled,
                                 QString &error) {
    AVFrame *frame;
    int got_output = 0;
    int ret;

    frame = get_audio_frame(ost);
    got_output |= !!frame;

    /* feed the data to lavr */
    if(frame) {
        ret = avresample_convert(ost->avr, nullptr, 0, 0,
                                 frame->extended_data, frame->linesize[0],
                                 frame->nb_samples);
        if(ret < 0) {
            error = "Error feeding audio data to the resampler";
            return false;
        }
    }

    while((frame && avresample_available(ost->avr) >= ost->frame->nb_samples) ||
          (!frame && avresample_get_out_samples(ost->avr, 0))) {
        /* when we pass a frame to the encoder, it may keep a reference to it
         * internally;
         * make sure we do not overwrite it here
         */
        if(av_frame_make_writable(ost->frame) < 0) {
            error = "Error making AVFrame writable";
            return false;
        }

        /* the difference between the two avresample calls here is that the
         * first one just reads the already converted data that is buffered in
         * the lavr output buffer, while the second one also flushes the
         * resampler */
        if(frame) {
            ret = avresample_read(ost->avr, ost->frame->extended_data,
                                  ost->frame->nb_samples);
        } else {
            ret = avresample_convert(ost->avr, ost->frame->extended_data,
                                     ost->frame->linesize[0], ost->frame->nb_samples,
                                     nullptr, 0, 0);
        }

        if(ret < 0) {
            error = "Error while resampling";
            return false;
        } else if(frame && ret != ost->frame->nb_samples) {
            error = "Too few samples returned from resampler";
            return false;
        }

        ost->frame->nb_samples = ret;

        ost->frame->pts        = ost->next_pts;
        ost->next_pts         += ost->frame->nb_samples;

        if(!encode_audio_frame(oc, ost, ost->frame, &got_output, error)) {
            return false;
        }
    }

    *audioEnabled = got_output;
    return true;
}

bool VideoEncoder::startEncodingNow(QString &error) {
    if(!mOutputFormat) {
        mOutputFormat = av_guess_format(nullptr, mPathByteArray.data(), nullptr);
        if(!mOutputFormat) {
            error = "No AVOutputFormat provided. "
                    "Could not guess AVOutputFormat from file extension";
            return false;
//            mOutputFormat = av_guess_format("mpeg", nullptr, nullptr);
        }
//        if(!mOutputFormat) {
//            fprintf(stderr, "MPEG format not available.\n");
//            return false;
//        }
    }
    mFormatContext = avformat_alloc_context();
    if(!mFormatContext) {
        error = "Error allocating AVFormatContext";
        return false;
    }
    mFormatContext->oformat = mOutputFormat;

    // add streams
    mHaveVideo = 0;
    mHaveAudio = 0;
    mEncodeVideo = 0;
    mEncodeAudio = 0;
    mVideoStream = { 0 };
    mAudioStream = { 0 };
    if(mOutputSettings.videoCodec != nullptr &&
       mOutputSettings.videoEnabled) {
        if(!add_video_stream(&mVideoStream, mFormatContext,
                             mOutputSettings, mRenderSettings,
                             error)) {
            return false;
        }
        mHaveVideo = 1;
        mEncodeVideo = 1;
    }
    if(mOutputFormat->audio_codec != AV_CODEC_ID_NONE  &&
            mOutputSettings.audioEnabled) {
        if(!add_audio_stream(&mAudioStream, mFormatContext,
                             mOutputSettings, error)) {
            return false;
        }
        mHaveAudio = 1;
        mEncodeAudio = 1;
    }
    if(!mHaveAudio && !mHaveVideo) {
        error = "No streams to render";
        return false;
    }
    // open streams
    if(mHaveVideo) {
        if(!open_video(mOutputSettings.videoCodec,
                       &mVideoStream, error)) {
            return false;
        }
    }
    if(mHaveAudio) {
        if(!open_audio(mOutputSettings.audioCodec,
                       &mAudioStream, error)) {
            return false;
        }
    }

    //av_dump_format(mFormatContext, 0, mPathByteArray.data(), 1);
    if(!(mOutputFormat->flags & AVFMT_NOFILE)) {
        if(avio_open(&mFormatContext->pb, mPathByteArray.data(), AVIO_FLAG_WRITE) < 0) {
            error = "Could not open " + QString(mPathByteArray.data());
            return false;
        }
    }

    if(avformat_write_header(mFormatContext, nullptr) < 0) {
        error = "Could not write header to " + QString(mPathByteArray.data());
        return false;
    }
    return true;
}

void VideoEncoder::startEncoding(RenderInstanceSettings *settings) {
    if(mCurrentlyEncoding) return;
    mRenderInstanceSettings = settings;
    mRenderInstanceSettings->renderingAboutToStart();
    mOutputSettings = mRenderInstanceSettings->getOutputRenderSettings();
    mRenderSettings = mRenderInstanceSettings->getRenderSettings();
    mPathByteArray = mRenderInstanceSettings->getOutputDestination().toLatin1();
    // get format from audio file

    mOutputFormat = mOutputSettings.outputFormat;
    QString error;
    if(startEncodingNow(error)) {
        mCurrentlyEncoding = true;
        mEncodingFinished = false;
        mRenderInstanceSettings->setCurrentState(
                    RenderInstanceSettings::RENDERING);
        mEmitter.encodingStarted();
    } else {
        mRenderInstanceSettings->setCurrentState(
                    RenderInstanceSettings::ERROR,
                    error);
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
    if(mHaveVideo) close_stream(mFormatContext, &mVideoStream);
    if(mHaveAudio) close_stream(mFormatContext, &mAudioStream);

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
    foreach(const stdsptr<ImageCacheContainer> &cont,
            _mContainers) {
        cont->setBlocked(false);
    }
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
            stdsptr<ImageCacheContainer> cacheCont =
                    _mContainers.at(_mCurrentContainerId);
            const int nFrames =
                    (cacheCont->getRange()*_mRenderRange).span();
            if(!write_video_frame(mFormatContext, &mVideoStream,
                                  cacheCont->getImageSk(),
                                  &mEncodeVideo, mUpdateError) ) {
                mUpdateFailed = true;
                return;
            }
            _mCurrentContainerFrame++;
            if(_mCurrentContainerFrame >= nFrames) {
                _mCurrentContainerId++;
                _mCurrentContainerFrame = 0;
                encodeVideoT = _mCurrentContainerId < _mContainers.count();
            }
        } else if(mEncodeAudio) {
            if(process_audio_stream(mFormatContext, &mAudioStream,
                                    &mEncodeAudio, mUpdateError)) {
                mUpdateFailed = true;
                return;
            }
        }
    }
}

void VideoEncoder::beforeProcessingStarted() {
    _ScheduledTask::beforeProcessingStarted();
    _mCurrentContainerId = 0;
    _mCurrentContainerFrame = 0;
    _mContainers.append(mNextContainers);
    _mRenderRange = {mRenderSettings.fMinFrame, mRenderSettings.fMaxFrame};
    mNextContainers.clear();
    if(!mCurrentlyEncoding) clearContainers();
}

void VideoEncoder::afterProcessingFinished() {
    bool firstT = true;
    for(int i = _mCurrentContainerId - 1; i >= 0; i--) {
        const auto &cont = _mContainers.at(i);
        if(firstT) {
            auto currCanvas = mRenderInstanceSettings->getTargetCanvas();
            currCanvas->setCurrentPreviewContainer(cont, true);
            firstT = false;
        } else {
            cont->setBlocked(false);
        }
        _mContainers.removeAt(i);
    }
    if(mEncodingFinished ||
       mInterruptEncoding ||
       mUpdateFailed ||
       (!mEncodeAudio && !mEncodeVideo)) {
        if(mInterruptEncoding) {
            mRenderInstanceSettings->setCurrentState(
                        RenderInstanceSettings::NONE);
            interrupEncoding();
            mInterruptEncoding = false;
        } else if(mUpdateFailed) {
            mRenderInstanceSettings->setCurrentState(
                        RenderInstanceSettings::ERROR,
                        mUpdateError);
            finishEncodingNow();
            mUpdateFailed = false;
            mEmitter.encodingFailed();
        } else {
            finishEncodingSuccess();
        }
    }
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
