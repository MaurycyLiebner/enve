#include "outputgenerator.h"

OutputGenerator::OutputGenerator()
{

}

static void video_encode_example(const char *filename,
                                 const int &codec_id) {
    AVCodec *codec;
    AVCodecContext *c= NULL;
    int i, ret, x, y, got_output;
    FILE *f;
    AVFrame *frame;
    AVPacket pkt;
    uint8_t endcode[] = { 0, 0, 1, 0xb7 };
    printf("Encode video file %s\n", filename);
    /* find the video encoder */
    codec = avcodec_find_encoder(static_cast<AVCodecID>(codec_id));
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }
    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }
    /* put sample parameters */
    c->bit_rate = 400000;
    /* resolution must be a multiple of two */
    c->width = 352;
    c->height = 288;
    /* frames per second */
    c->time_base = av_make_q(1, 25);
    /* emit one intra frame every ten frames
     * check frame pict_type before passing frame
     * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
     * then gop_size is ignored and the output of encoder
     * will always be I frame irrespective to gop_size
     */
    c->gop_size = 10;
    c->max_b_frames = 1;
    c->pix_fmt = AV_PIX_FMT_YUV420P;
    if (codec_id == AV_CODEC_ID_H264)
        av_opt_set(c->priv_data, "preset", "slow", 0);
    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }
    f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "Could not open %s\n", filename);
        exit(1);
    }
    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }
    frame->format = c->pix_fmt;
    frame->width  = c->width;
    frame->height = c->height;
    /* the image can be allocated by any means and av_image_alloc() is
     * just the most convenient way if av_malloc() is to be used */
    ret = av_image_alloc(frame->data, frame->linesize, c->width, c->height,
                         c->pix_fmt, 32);
    if (ret < 0) {
        fprintf(stderr, "Could not allocate raw picture buffer\n");
        exit(1);
    }
    /* encode 1 second of video */
    for (i = 0; i < 25; i++) {
        av_init_packet(&pkt);
        pkt.data = NULL;    // packet data will be allocated by the encoder
        pkt.size = 0;
        fflush(stdout);
        /* prepare a dummy image */
        /* Y */
        for (y = 0; y < c->height; y++) {
            for (x = 0; x < c->width; x++) {
                frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
            }
        }
        /* Cb and Cr */
        for (y = 0; y < c->height/2; y++) {
            for (x = 0; x < c->width/2; x++) {
                frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
                frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
            }
        }
        frame->pts = i;
        /* encode the image */
        ret = avcodec_encode_video2(c, &pkt, frame, &got_output);
        if (ret < 0) {
            fprintf(stderr, "Error encoding frame\n");
            exit(1);
        }
        if (got_output) {
            printf("Write frame %3d (size=%5d)\n", i, pkt.size);
            fwrite(pkt.data, 1, pkt.size, f);
            av_packet_unref(&pkt);
        }
    }
    /* get the delayed frames */
    for (got_output = 1; got_output; i++) {
        fflush(stdout);
        ret = avcodec_encode_video2(c, &pkt, NULL, &got_output);
        if (ret < 0) {
            fprintf(stderr, "Error encoding frame\n");
            exit(1);
        }
        if (got_output) {
            printf("Write frame %3d (size=%5d)\n", i, pkt.size);
            fwrite(pkt.data, 1, pkt.size, f);
            av_packet_unref(&pkt);
        }
    }
    /* add sequence end code to have a real MPEG file */
    fwrite(endcode, 1, sizeof(endcode), f);
    fclose(f);
    avcodec_close(c);
    av_free(c);
    av_freep(&frame->data[0]);
    av_frame_free(&frame);
    printf("\n");
}

/* check that a given sample format is supported by the encoder */
static int check_sample_fmt(AVCodec *codec, enum AVSampleFormat sample_fmt)
{
    const enum AVSampleFormat *p = codec->sample_fmts;
    while (*p != AV_SAMPLE_FMT_NONE) {
        if (*p == sample_fmt)
            return 1;
        p++;
    }
    return 0;
}

/* just pick the highest supported samplerate */
static int select_sample_rate(AVCodec *codec)
{
    const int *p;
    int best_samplerate = 0;
    if (!codec->supported_samplerates)
        return 44100;
    p = codec->supported_samplerates;
    while (*p) {
        best_samplerate = FFMAX(*p, best_samplerate);
        p++;
    }
    return best_samplerate;
}
/* select layout with the highest channel count */
static int select_channel_layout(AVCodec *codec)
{
    const uint64_t *p;
    uint64_t best_ch_layout = 0;
    int best_nb_channels   = 0;
    if (!codec->channel_layouts)
        return AV_CH_LAYOUT_STEREO;
    p = codec->channel_layouts;
    while (*p) {
        int nb_channels = av_get_channel_layout_nb_channels(*p);
        if (nb_channels > best_nb_channels) {
            best_ch_layout    = *p;
            best_nb_channels = nb_channels;
        }
        p++;
    }
    return best_ch_layout;
}

static void audio_encode_example(const char *filename) {
    AVCodec *codec;
    AVCodecContext *c= NULL;
    AVFrame *frame;
    AVPacket pkt;
    int i, j, k, ret, got_output;
    int buffer_size;
    FILE *f;
    uint16_t *samples;
    float t, tincr;
    printf("Encode audio file %s\n", filename);
    /* find the MP2 encoder */
    codec = avcodec_find_encoder(AV_CODEC_ID_MP2);
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }
    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate audio codec context\n");
        exit(1);
    }
    /* put sample parameters */
    c->bit_rate = 64000;
    /* check that the encoder supports s16 pcm input */
    c->sample_fmt = AV_SAMPLE_FMT_S16;
    if (!check_sample_fmt(codec, c->sample_fmt)) {
        fprintf(stderr, "Encoder does not support sample format %s",
                av_get_sample_fmt_name(c->sample_fmt));
        exit(1);
    }
    /* select other audio parameters supported by the encoder */
    c->sample_rate    = select_sample_rate(codec);
    c->channel_layout = select_channel_layout(codec);
    c->channels       = av_get_channel_layout_nb_channels(c->channel_layout);
    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }
    f = fopen(filename, "wb");
    if (!f) {
        fprintf(stderr, "Could not open %s\n", filename);
        exit(1);
    }
    /* frame containing input raw audio */
    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate audio frame\n");
        exit(1);
    }
    frame->nb_samples     = c->frame_size;
    frame->format         = c->sample_fmt;
    frame->channel_layout = c->channel_layout;
    /* the codec gives us the frame size, in samples,
     * we calculate the size of the samples buffer in bytes */
    buffer_size = av_samples_get_buffer_size(NULL, c->channels, c->frame_size,
                                             c->sample_fmt, 0);
    if (buffer_size < 0) {
        fprintf(stderr, "Could not get sample buffer size\n");
        exit(1);
    }
    samples = (uint16_t*)av_malloc(buffer_size);
    if (!samples) {
        fprintf(stderr, "Could not allocate %d bytes for samples buffer\n",
                buffer_size);
        exit(1);
    }
    /* setup the data pointers in the AVFrame */
    ret = avcodec_fill_audio_frame(frame, c->channels, c->sample_fmt,
                                   (const uint8_t*)samples, buffer_size, 0);
    if (ret < 0) {
        fprintf(stderr, "Could not setup audio frame\n");
        exit(1);
    }
    /* encode a single tone sound */
    t = 0;
    tincr = 2 * M_PI * 440.0 / c->sample_rate;
    for (i = 0; i < 200; i++) {
        av_init_packet(&pkt);
        pkt.data = NULL; // packet data will be allocated by the encoder
        pkt.size = 0;
        for (j = 0; j < c->frame_size; j++) {
            samples[2*j] = (int)(sin(t) * 10000);
            for (k = 1; k < c->channels; k++)
                samples[2*j + k] = samples[2*j];
            t += tincr;
        }
        /* encode the samples */
        ret = avcodec_encode_audio2(c, &pkt, frame, &got_output);
        if (ret < 0) {
            fprintf(stderr, "Error encoding audio frame\n");
            exit(1);
        }
        if (got_output) {
            fwrite(pkt.data, 1, pkt.size, f);
            av_packet_unref(&pkt);
        }
    }
    /* get the delayed frames */
    for (got_output = 1; got_output; i++) {
        ret = avcodec_encode_audio2(c, &pkt, NULL, &got_output);
        if (ret < 0) {
            fprintf(stderr, "Error encoding frame\n");
            exit(1);
        }
        if (got_output) {
            fwrite(pkt.data, 1, pkt.size, f);
            av_packet_unref(&pkt);
        }
    }
    fclose(f);
    av_freep(&samples);
    av_frame_free(&frame);
    avcodec_close(c);
    av_free(c);
}

