#ifndef VIDEOCACHEHANDLER_H
#define VIDEOCACHEHANDLER_H
#include "animationcachehandler.h"
#include "Boxes/rendercachehandler.h"

extern "C" {
    #include <libavutil/opt.h>
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswresample/swresample.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}

struct VideoStreamsData {
    bool fOpened = false;
    AVFormatContext *fFormatContext = nullptr;
    int fVideoStreamIndex = -1;
    AVStream * fVideoStream = nullptr;
    AVPacket * fPacket = nullptr;
    AVFrame *fDecodedFrame = nullptr;
    AVCodecContext * fCodecContext = nullptr;
    struct SwsContext * fSwsContext = nullptr;

    void open(const char * const path) {
        fFormatContext = avformat_alloc_context();
        if(!fFormatContext) {
            fprintf(stderr, "Error allocating AVFormatContext\n");
            return;
        }
        if(avformat_open_input(&fFormatContext, path, nullptr, nullptr) != 0) {
            fprintf(stderr, "Could not open file '%s'\n", path);
            return;
        }
        if(avformat_find_stream_info(fFormatContext, nullptr) < 0) {
            fprintf(stderr, "Could not retrieve stream info from file '%s'\n", path);
            return;
        }

        // Find the index of the first audio stream
        int videoStreamIndex = -1;
        AVCodecParameters *codecPars = nullptr;
        AVCodec *codec = nullptr;
        AVStream *videoStream = nullptr;
        for(uint i = 0; i < fFormatContext->nb_streams; i++) {
            AVStream * const  streamT = fFormatContext->streams[i];
            AVCodecParameters * const codecParsT = streamT->codecpar;
            const AVMediaType &mediaType = codecParsT->codec_type;
            if(mediaType == AVMEDIA_TYPE_VIDEO) {
                videoStreamIndex = static_cast<int>(i);
                codecPars = codecParsT;
                codec = avcodec_find_decoder(codecPars->codec_id);
                videoStream = fFormatContext->streams[videoStreamIndex];
                break;
            }
        }
        if(videoStreamIndex == -1) {
            fprintf(stderr, "Could not retrieve video stream from file '%s'\n", path);
            return;
        }

        if(!codec) {
            fprintf(stderr, "Unsuported codec\n");
            return;
        }
        fCodecContext = avcodec_alloc_context3(codec);
        if(!fCodecContext) {
            fprintf(stderr, "Error allocating AVCodecContext\n");
            return;
        }
        if(avcodec_parameters_to_context(fCodecContext, codecPars) < 0) {
            fprintf(stderr, "Failed to copy codec params to codec context\n");
            return;
        }

        if(avcodec_open2(fCodecContext, codec, nullptr) < 0 ) {
            fprintf(stderr, "Failed to open codec\n");
            return;
        }
        fSwsContext =
              sws_getContext(fCodecContext->width, fCodecContext->height,
                             fCodecContext->pix_fmt,
                             fCodecContext->width, fCodecContext->height,
                             AV_PIX_FMT_BGRA, SWS_BICUBIC, nullptr, nullptr, nullptr);

        fPacket = av_packet_alloc();
        if(!fPacket) {
            fprintf(stderr, "Error allocating AVPacket\n");
            return;
        }
        fDecodedFrame = av_frame_alloc();
        if(!fDecodedFrame) {
            fprintf(stderr, "Error allocating AVFrame\n");
            return;
        }

        fOpened = true;
    }

    void close() {
        fOpened = false;

        avformat_close_input(&fFormatContext);
        av_packet_free(&fPacket);
        sws_freeContext(fSwsContext);
        avcodec_close(fCodecContext);
        av_frame_free(&fDecodedFrame);
        avformat_free_context(fFormatContext);

        fFormatContext = nullptr;
        fVideoStreamIndex = -1;
        fVideoStream = nullptr;
        fPacket = nullptr;
        fDecodedFrame = nullptr;
        fCodecContext = nullptr;
        fSwsContext = nullptr;
    }
};

class VideoFrameLoader : public _HDDTask {
protected:
    VideoFrameLoader(const VideoStreamsData * const openedVideo,
                     const int& frameId, const qreal& fps) :
        mOpenedVideo(openedVideo), mFrameId(frameId), mFps(fps) {

    }
public:
    void _processUpdate() {
        readFrame();
    }
private:
    void readFrame() {
        if(!mOpenedVideo->fOpened)
            RuntimeThrow("Cannot read frame from closed VideoStream");
        const auto formatContext = mOpenedVideo->fFormatContext;
        const auto videoStreamIndex = mOpenedVideo->fVideoStreamIndex;
        const auto videoStream = mOpenedVideo->fVideoStream;
        const auto packet = mOpenedVideo->fPacket;
        const auto decodedFrame = mOpenedVideo->fDecodedFrame;
        const auto codecContext = mOpenedVideo->fCodecContext;
        const auto swsContext = mOpenedVideo->fSwsContext;

        bool frameReceived = false;
        const int tsms = qRound(mFrameId * 1000 / mFps);

        const int64_t frame = av_rescale(tsms, videoStream->time_base.den,
                                         videoStream->time_base.num)/1000;

        if(mFrameId != 0) {
            if(avformat_seek_file(formatContext, videoStreamIndex, 0,
                                  frame, frame, AVSEEK_FLAG_FRAME) < 0) {
                return;
            }
        }

        avcodec_flush_buffers(codecContext);

        int64_t pts = 0;

        while(true) {
            if(av_read_frame(formatContext, packet) < 0) {
                break;
            } else {
                int response = 0;
                if(packet->stream_index == videoStreamIndex) {
                    response = avcodec_send_packet(codecContext, packet);
                    if(response < 0) {
                        fprintf(stderr, "Sending packet to the decoder failed\n");
                        return;
                    }
                    response = avcodec_receive_frame(codecContext, decodedFrame);
                    if(response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
                        av_packet_unref(packet);
                        continue;
                    } else if(response < 0) {
                        fprintf(stderr, "Did not receive frame from the decoder\n");
                        return;
                    }
                    av_packet_unref(packet);
                } else {
                    av_packet_unref(packet);
                    continue;
                }
                //if(frameId == 0) break;
            }

            // calculate PTS:
            pts = av_frame_get_best_effort_timestamp(decodedFrame);
            pts = av_rescale_q(pts, videoStream->time_base, AV_TIME_BASE_Q);
            const int currFrame = qRound(pts/1000000.*mFps);
            if(currFrame == mFrameId) {
//                qDebug() << pts/1000 << tsms;
//                qDebug() << "for" << frameId << "received" << pts*mUpdateFps/1000000;
//                qDebug() << "seeked" << frame;
                frameReceived = true;
                break;
            }
            av_frame_unref(decodedFrame);
        }

    // SKIA

        if(frameReceived) {
            const auto info = SkiaHelpers::getPremulBGRAInfo(
                        codecContext->width, codecContext->height);
            SkBitmap bitmap;
            bitmap.allocPixels(info);

            SkPixmap pixmap;
            bitmap.peekPixels(&pixmap);

            void * const addr = pixmap.writable_addr();
            uint8_t *dstSk[] = { static_cast<unsigned char*>(addr) };
            int linesizesSk[4];

            av_image_fill_linesizes(linesizesSk, AV_PIX_FMT_BGRA,
                                    decodedFrame->width);

            sws_scale(swsContext, decodedFrame->data, decodedFrame->linesize,
                      0, codecContext->height, dstSk, linesizesSk);

            mLoadedFrame = SkImage::MakeFromBitmap(bitmap);
        } else {
            mLoadedFrame.reset();
        }
        av_frame_unref(decodedFrame);
        av_packet_unref(packet);
    }

    const VideoStreamsData * const mOpenedVideo;
    const int mFrameId;
    const qreal mFps;
    sk_sp<SkImage> mLoadedFrame;
};

class VideoCacheHandler : public AnimationCacheHandler {
    friend class StdSelfRef;
protected:
    VideoCacheHandler(const QString &filePath);

    void updateFrameCount();
public:
    sk_sp<SkImage> getFrameAtFrame(const int &relFrame);
    sk_sp<SkImage> getFrameAtOrBeforeFrame(const int &relFrame);
    _ScheduledTask *scheduleFrameLoad(const int &frame);

    void beforeProcessingStarted();

    void _processUpdate();

    void afterProcessingFinished();

    void clearCache();

    void replace();

    const qreal &getFps();
protected:
    int mTimeBaseDen = 1;
    int mTimeBaseNum = 24;
    int mUpdateTimeBaseDen = 1;
    int mUpdateTimeBaseNum = 24;

    qreal mFps = 24;
    qreal mUpdateFps = 24;

    QList<int> mFramesLoadScheduled;

    QList<int> mFramesBeingLoadedGUI;
    QList<int> mFramesBeingLoaded;
    QList<sk_sp<SkImage>> mLoadedFrames;

    RenderCacheHandler mFramesCache;
};

#endif // VIDEOCACHEHANDLER_H
