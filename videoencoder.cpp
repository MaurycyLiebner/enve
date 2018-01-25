#include "videoencoder.h"
#include <QByteArray>
VideoEncoder *VideoEncoder::mVideoEncoderInstance = NULL;

VideoEncoder::VideoEncoder() {
    mVideoEncoderInstance = this;
}

void VideoEncoder::addImage(const sk_sp<SkImage> &img) {
    if(img.get() == NULL) return;
    SkPixmap pix;
    img->peekPixels(&pix);
    mImages.append(SkImage::MakeRasterCopy(pix));
    addScheduler();
}

static AVFrame *alloc_picture(enum AVPixelFormat pix_fmt,
                              int width, int height) {
    AVFrame *picture;
    int ret;

    picture = av_frame_alloc();
    if(!picture) return NULL;

    picture->format = pix_fmt;
    picture->width  = width;
    picture->height = height;

    /* allocate the buffers for the frame data */
    ret = av_frame_get_buffer(picture, 32);
    if(ret < 0) {
        fprintf(stderr, "Could not allocate frame data.\n");
        exit(1);
    }

    return picture;
}

static void open_video(AVFormatContext *oc, OutputStream *ost) {
    AVCodecContext *c;
    int ret;

    c = ost->enc;

    /* open the codec */
    if(avcodec_open2(c, NULL, NULL) < 0) {
       fprintf(stderr, "could not open codec\n");
       exit(1);
    }

    /* Allocate the encoded raw picture. */
    ost->frame = alloc_picture(c->pix_fmt, c->width, c->height);
    if(!ost->frame) {
        fprintf(stderr, "Could not allocate picture\n");
       exit(1);
    }

    /* copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(ost->st->codecpar, c);
    if (ret < 0) {
        fprintf(stderr, "Could not copy the stream parameters\n");
        exit(1);
    }
}

static void add_video_stream(OutputStream *ost,
                             AVFormatContext *oc,
                             const RenderInstanceSettings &settings) {
    AVCodecContext *c;
    AVCodec *codec = settings.getVideoCodec();

//    if(codec == NULL) {
//        /* find the video encoder */
//        codec = avcodec_find_encoder(codec_id);
//        if(!codec) {
//            fprintf(stderr, "codec not found\n");
//            exit(1);
//        }
//    }

    ost->st = avformat_new_stream(oc, NULL);
    if(!ost->st) {
        fprintf(stderr, "Could not alloc stream\n");
        exit(1);
    }

    c = avcodec_alloc_context3(codec);
    if(!c) {
        fprintf(stderr, "Could not alloc an encoding context\n");
        exit(1);
    }
    ost->enc = c;

    /* Put sample parameters. */
    c->bit_rate = settings.getVideoBitrate();//settings.getVideoBitrate();
    /* Resolution must be a multiple of two. */
    c->width    = settings.getVideoWidth();
    c->height   = settings.getVideoHeight();
    /* timebase: This is the fundamental unit of time (in seconds) in terms
     * of which frame timestamps are represented. For fixed-fps content,
     * timebase should be 1/framerate and timestamp increments should be
     * identical to 1. */
    ost->st->time_base = settings.getTimeBase();
    c->time_base       = ost->st->time_base;

    c->gop_size      = 12; /* emit one intra frame every twelve frames at most */
    c->pix_fmt       = settings.getVideoPixelFormat();//BGRA;
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
    if(oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
}

/* Prepare a dummy image. */
static void copyImageToFrame(AVFrame *pict,
                           const sk_sp<SkImage> &skiaImg,
                           int width, int height) {
    int ret;

    /* when we pass a frame to the encoder, it may keep a reference to it
     * internally;
     * make sure we do not overwrite it here
     */
    ret = av_frame_make_writable(pict);
    if(ret < 0) exit(1);

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
}

static AVFrame *get_video_frame(OutputStream *ost,
                                const sk_sp<SkImage> &image) {
    AVCodecContext *c = ost->enc;

    /* check if we want to generate more frames */
//    if (av_compare_ts(ost->next_pts, c->time_base,
//                      STREAM_DURATION, (AVRational){ 1, 1 }) >= 0)
//        return NULL;

    if(c->pix_fmt != AV_PIX_FMT_BGRA) {
        /* as we only generate a rgba picture, we must convert it
         * to the codec pixel format if needed */
        if (!ost->sws_ctx) {
            ost->sws_ctx = sws_getContext(c->width, c->height,
                                          AV_PIX_FMT_BGRA,
                                          c->width, c->height,
                                          c->pix_fmt,
                                          SWS_BICUBIC, NULL, NULL, NULL);
            if (!ost->sws_ctx) {
                fprintf(stderr,
                        "Cannot initialize the conversion context\n");
                exit(1);
            }
        }
        SkPixmap pixmap;
        image->peekPixels(&pixmap);
        uint8_t *dstSk[] = {(unsigned char*)pixmap.writable_addr()};
        int linesizesSk[4];

        av_image_fill_linesizes(linesizesSk,
                                AV_PIX_FMT_BGRA,
                                image->width());
        sws_scale(ost->sws_ctx, dstSk,
                  linesizesSk, 0, c->height, ost->frame->data,
                  ost->frame->linesize);
    } else {
        copyImageToFrame(ost->frame, image, c->width, c->height);
    }

    ost->frame->pts = ost->next_pts++;

    return ost->frame;
}

static int write_video_frame(AVFormatContext *oc,
                             OutputStream *ost,
                             const sk_sp<SkImage> &image) {
    int ret;
    AVCodecContext *c;
    AVFrame *frame;

    c = ost->enc;

    frame = get_video_frame(ost, image);

    /* encode the image */
    ret = avcodec_send_frame(c, frame);
    if (ret < 0) {
        fprintf(stderr, "Error submitting a frame for encoding\n");
        exit(1);
    }

    while (ret >= 0) {
        AVPacket pkt = { 0 };

        av_init_packet(&pkt);

        ret = avcodec_receive_packet(c, &pkt);
        if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
            fprintf(stderr, "Error encoding a video frame\n");
            exit(1);
        } else if (ret >= 0) {
            av_packet_rescale_ts(&pkt, c->time_base, ost->st->time_base);
            pkt.stream_index = ost->st->index;

            /* Write the compressed frame to the media file. */
            ret = av_interleaved_write_frame(oc, &pkt);
            if (ret < 0) {
                fprintf(stderr, "Error while writing video frame\n");
                exit(1);
            }
        }
    }

    return ret == AVERROR_EOF;
}

static void close_stream(AVFormatContext *oc, OutputStream *ost) {
    avcodec_free_context(&ost->enc);
    av_frame_free(&ost->frame);
    sws_freeContext(ost->sws_ctx);
    avresample_free(&ost->avr);
}

static void add_audio_stream(OutputStream *ost,
                             AVFormatContext *oc,
                             const RenderInstanceSettings &settings) {
    AVCodecContext *c;
    AVCodec *codec = settings.getAudioCodec();
    int ret;

//    /* find the audio encoder */
//    codec = avcodec_find_encoder(codec_id);
//    if (!codec) {
//        fprintf(stderr, "codec not found\n");
//        exit(1);
//    }

    ost->st = avformat_new_stream(oc, NULL);
    if (!ost->st) {
        fprintf(stderr, "Could not alloc stream\n");
        exit(1);
    }

    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not alloc an encoding context\n");
        exit(1);
    }
    ost->enc = c;

    /* put sample parameters */
    c->sample_fmt     = settings.getAudioSampleFormat();
    c->sample_rate    = settings.getAudioSampleRate();
    c->channel_layout = settings.getAudioChannelsLayout();
    c->channels       = av_get_channel_layout_nb_channels(c->channel_layout);
    c->bit_rate       = settings.getAudioBitrate();

    ost->st->time_base = (AVRational){ 1, c->sample_rate };

    // some formats want stream headers to be separate
    if (oc->oformat->flags & AVFMT_GLOBALHEADER)
        c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    /* initialize sample format conversion;
     * to simplify the code, we always pass the data through lavr, even
     * if the encoder supports the generated format directly -- the price is
     * some extra data copying;
     */
    ost->avr = avresample_alloc_context();
    if (!ost->avr) {
        fprintf(stderr, "Error allocating the resampling context\n");
        exit(1);
    }

    av_opt_set_int(ost->avr, "in_sample_fmt",      AV_SAMPLE_FMT_S16,   0); // !!!
    av_opt_set_int(ost->avr, "in_sample_rate",     44100,               0);
    av_opt_set_int(ost->avr, "in_channel_layout",  AV_CH_LAYOUT_STEREO, 0);
    av_opt_set_int(ost->avr, "out_sample_fmt",     c->sample_fmt,       0);
    av_opt_set_int(ost->avr, "out_sample_rate",    c->sample_rate,      0);
    av_opt_set_int(ost->avr, "out_channel_layout", c->channel_layout,   0);

    ret = avresample_open(ost->avr);
    if (ret < 0) {
        fprintf(stderr, "Error opening the resampling context\n");
        exit(1);
    }
}

static AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt,
                                  uint64_t channel_layout,
                                  int sample_rate, int nb_samples) {
    AVFrame *frame = av_frame_alloc();
    int ret;

    if (!frame) {
        fprintf(stderr, "Error allocating an audio frame\n");
        exit(1);
    }

    frame->format = sample_fmt;
    frame->channel_layout = channel_layout;
    frame->sample_rate = sample_rate;
    frame->nb_samples = nb_samples;

    if (nb_samples) {
        ret = av_frame_get_buffer(frame, 0);
        if (ret < 0) {
            fprintf(stderr, "Error allocating an audio buffer\n");
            exit(1);
        }
    }

    return frame;
}

static void open_audio(AVFormatContext *oc, OutputStream *ost) {
    AVCodecContext *c;
    int nb_samples, ret;

    c = ost->enc;

    /* open it */
    if(avcodec_open2(c, NULL, NULL) < 0) {
        fprintf(stderr, "could not open codec\n");
        exit(1);
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
                                   c->sample_rate, nb_samples);
    ost->tmp_frame = alloc_audio_frame(AV_SAMPLE_FMT_S16, AV_CH_LAYOUT_STEREO,
                                       44100, nb_samples); // !!!

    /* copy the stream parameters to the muxer */
    ret = avcodec_parameters_from_context(ost->st->codecpar, c);
    if(ret < 0) {
        fprintf(stderr, "Could not copy the stream parameters\n");
        exit(1);
    }
}

/* Prepare a 16 bit dummy audio frame of 'frame_size' samples and
 * 'nb_channels' channels. */
static AVFrame *get_audio_frame(OutputStream *ost) {
    AVFrame *frame = ost->tmp_frame;
    int j, i, v;
    int16_t *q = (int16_t*)frame->data[0];

    /* check if we want to generate more frames */
//    if(av_compare_ts(ost->next_pts, ost->enc->time_base,
//                     STREAM_DURATION, (AVRational){ 1, 1 }) >= 0)
//        return NULL;


    for(j = 0; j < frame->nb_samples; j++) {
        v = (int)(sin(ost->t) * 10000);
        for(i = 0; i < ost->enc->channels; i++)
            *q++ = v;
        ost->t     += ost->tincr;
        ost->tincr += ost->tincr2;
    }

    return frame;
}


/* if a frame is provided, send it to the encoder, otherwise flush the encoder;
 * return 1 when encoding is finished, 0 otherwise
 */
static int encode_audio_frame(AVFormatContext *oc,
                              OutputStream *ost,
                              AVFrame *frame) {
    int ret;

    ret = avcodec_send_frame(ost->enc, frame);
    if(ret < 0) {
        fprintf(stderr, "Error submitting a frame for encoding\n");
        exit(1);
    }

    while(ret >= 0) {
        AVPacket pkt = { 0 }; // data and size must be 0;

        av_init_packet(&pkt);

        ret = avcodec_receive_packet(ost->enc, &pkt);
        if(ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF) {
            fprintf(stderr, "Error encoding a video frame\n");
            exit(1);
        } else if(ret >= 0) {
            av_packet_rescale_ts(&pkt, ost->enc->time_base, ost->st->time_base);
            pkt.stream_index = ost->st->index;

            /* Write the compressed frame to the media file. */
            ret = av_interleaved_write_frame(oc, &pkt);
            if(ret < 0) {
                fprintf(stderr, "Error while writing video frame\n");
                exit(1);
            }
        }
    }

    return ret == AVERROR_EOF;
}

static int process_audio_stream(AVFormatContext *oc, OutputStream *ost) {
    AVFrame *frame;
    int got_output = 0;
    int ret;

    frame = get_audio_frame(ost);
    got_output |= !!frame;

    /* feed the data to lavr */
    if (frame) {
        ret = avresample_convert(ost->avr, NULL, 0, 0,
                                 frame->extended_data, frame->linesize[0],
                                 frame->nb_samples);
        if (ret < 0) {
            fprintf(stderr, "Error feeding audio data to the resampler\n");
            exit(1);
        }
    }

    while ((frame && avresample_available(ost->avr) >= ost->frame->nb_samples) ||
           (!frame && avresample_get_out_samples(ost->avr, 0))) {
        /* when we pass a frame to the encoder, it may keep a reference to it
         * internally;
         * make sure we do not overwrite it here
         */
        ret = av_frame_make_writable(ost->frame);
        if (ret < 0)
            exit(1);

        /* the difference between the two avresample calls here is that the
         * first one just reads the already converted data that is buffered in
         * the lavr output buffer, while the second one also flushes the
         * resampler */
        if (frame) {
            ret = avresample_read(ost->avr, ost->frame->extended_data,
                                  ost->frame->nb_samples);
        } else {
            ret = avresample_convert(ost->avr, ost->frame->extended_data,
                                     ost->frame->linesize[0], ost->frame->nb_samples,
                                     NULL, 0, 0);
        }

        if (ret < 0) {
            fprintf(stderr, "Error while resampling\n");
            exit(1);
        } else if (frame && ret != ost->frame->nb_samples) {
            fprintf(stderr, "Too few samples returned from lavr\n");
            exit(1);
        }

        ost->frame->nb_samples = ret;

        ost->frame->pts        = ost->next_pts;
        ost->next_pts         += ost->frame->nb_samples;

        got_output |= encode_audio_frame(oc, ost, ret ? ost->frame : NULL);
    }

    return !got_output;
}

void VideoEncoder::startEncoding(const RenderInstanceSettings &settings) {
    if(mStartedEncoding) return;
    mStartedEncoding = true;
    mEncodingInterrupted = false;
    mRenderInstanceSettings = settings;
    mRenderInstanceSettings.updateRenderVars();
    mPathByteArray = mRenderInstanceSettings.getOutputDestination().toLatin1();
    // get format from audio file

    mOutputFormat = settings.getOutputFormat();
    if(mOutputFormat == NULL) {
        mOutputFormat = av_guess_format(NULL, mPathByteArray.data(), NULL);
        if(mOutputFormat == NULL) {
            fprintf(stderr, "Could not guess AVOutputFormat from file extension: using MPEG.\n");
            mOutputFormat = av_guess_format("mpeg", NULL, NULL);
        }
        if(mOutputFormat == NULL) {
            fprintf(stderr, "MPEG format not available.\n");
            return;// -1;
        }
    }
    mFormatContext = avformat_alloc_context();
    if(!mFormatContext) {
        fprintf(stderr, "Error allocating AVFormatContext\n");
        return;// -1;
    }
    mFormatContext->oformat = mOutputFormat;

    // add streams
    mHaveVideo = 0;
    mHaveAudio = 0;
    mEncodeVideo = 0;
    mEncodeAudio = 0;
    mVideoStream = { 0 };
    mAudioStream = { 0 };
    if(settings.getVideoCodec() != NULL &&
        settings.getVideoEnabled()) {
        add_video_stream(&mVideoStream, mFormatContext,
                         mRenderInstanceSettings);
        mHaveVideo = 1;
        mEncodeVideo = 1;
    }
    if(mOutputFormat->audio_codec != AV_CODEC_ID_NONE  &&
            settings.getAudioEnabled()) {
        add_audio_stream(&mAudioStream, mFormatContext,
                         mRenderInstanceSettings);
        mHaveAudio = 1;
        mEncodeAudio = 1;
    }
    // open streams
    if(mHaveVideo) open_video(mFormatContext, &mVideoStream);
    if(mHaveAudio) open_video(mFormatContext, &mAudioStream);

    av_dump_format(mFormatContext, 0, mPathByteArray.data(), 1);
    if(!(mOutputFormat->flags & AVFMT_NOFILE)) {
        if(avio_open(&mFormatContext->pb, mPathByteArray.data(), AVIO_FLAG_WRITE) < 0) {
            fprintf(stderr, "Could not open '%s'\n", mPathByteArray.data());
            return; // -1;
        }
    }

    if(avformat_write_header(mFormatContext, NULL) < 0) {
        fprintf(stderr, "Could not open '%s'\n", mPathByteArray.data());
        return; // -1;
    }
}

void VideoEncoder::finishEncoding() {
    if(!mStartedEncoding) return;

    /* Write the trailer, if any. The trailer must be written before you
     * close the CodecContexts open when you wrote the header; otherwise
     * av_write_trailer() may try to use memory that was freed on
     * av_codec_close(). */
    av_write_trailer(mFormatContext);

    /* Close each codec. */
    if(mHaveVideo) close_stream(mFormatContext, &mVideoStream);
    if(mHaveAudio) close_stream(mFormatContext, &mAudioStream);


    if(!(mOutputFormat->flags & AVFMT_NOFILE)) avio_close(mFormatContext->pb);
    avformat_free_context(mFormatContext);

    mEncodeAudio = false;
    mEncodeVideo = false;
    mStartedEncoding = false;
    mImages.clear();
}

void VideoEncoder::processUpdate() {
    bool encodeVideoT = true; // local encode
    bool encodeAudioT = true; // local encode
    while((mEncodeVideo && encodeVideoT) || (mEncodeAudio && encodeAudioT)) {
        if(mEncodeVideo && encodeVideoT &&
           (!mEncodeAudio || av_compare_ts(mVideoStream.next_pts, mVideoStream.enc->time_base,
                                           mAudioStream.next_pts, mAudioStream.enc->time_base) <= 0)) {
            mEncodeVideo = !write_video_frame(mFormatContext, &mVideoStream,
                                              mUpdateImages.takeFirst());
            encodeVideoT = !mUpdateImages.isEmpty();
        } else {
            mEncodeAudio = !process_audio_stream(mFormatContext, &mAudioStream);
        }
    }
}

void VideoEncoder::beforeUpdate() {
    Updatable::beforeUpdate();
    mUpdateImages = mImages;
    mImages.clear();
}

void VideoEncoder::afterUpdate() {
    if(mEncodingFinished || (!mEncodeAudio && !mEncodeVideo)) {
        finishEncoding();
    }
    if(mEncodingInterrupted) {
        finishEncoding();
        if(mNewEncodingPlanned) {
            mNewEncodingPlanned = false;
            startEncoding(mWaitingRenderInstanceSettings);
        }
    }
    Updatable::afterUpdate();
}

void VideoEncoder::finishEncodingStatic() {
    mVideoEncoderInstance->finishCurrentEncoding();
}

void VideoEncoder::interruptEncodingStatic() {
    mVideoEncoderInstance->interruptCurrentEncoding();
}

void VideoEncoder::startEncodingStatic(const RenderInstanceSettings &settings) {
    mVideoEncoderInstance->startNewEncoding(settings);
}

void VideoEncoder::addImageToEncoderStatic(const sk_sp<SkImage> &img) {
    mVideoEncoderInstance->addImage(img);
}
