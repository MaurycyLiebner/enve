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
    QString fPath;
    bool fOpened = false;
    qreal fFps = 0;
    int fTimeBaseNum = 0;
    int fTimeBaseDen = 1;
    int fFrameCount = 0;
    AVFormatContext *fFormatContext = nullptr;
    int fVideoStreamIndex = -1;
    AVStream * fVideoStream = nullptr;
    AVPacket * fPacket = nullptr;
    AVFrame *fDecodedFrame = nullptr;
    AVCodecContext * fCodecContext = nullptr;
    struct SwsContext * fSwsContext = nullptr;

    void open(const QString& path) {
        fPath = path;
        open();
    }

    void close() {
        fOpened = false;

        if(fFormatContext) avformat_close_input(&fFormatContext);
        if(fPacket) av_packet_free(&fPacket);
        if(fSwsContext) sws_freeContext(fSwsContext);
        if(fCodecContext) avcodec_close(fCodecContext);
        if(fDecodedFrame) av_frame_free(&fDecodedFrame);
        if(fFormatContext) avformat_free_context(fFormatContext);

        fFormatContext = nullptr;
        fVideoStreamIndex = -1;
        fVideoStream = nullptr;
        fPacket = nullptr;
        fDecodedFrame = nullptr;
        fCodecContext = nullptr;
        fSwsContext = nullptr;
    }

private:
    void open() {
        const QByteArray stringByteArray = fPath.toLatin1();
        const char * const path = stringByteArray.constData();
        try {
            open(path);
        } catch(...) {
            close();
            RuntimeThrow("Failed to setup video stream for '" + path + "'.");
        }
    }

    void open(const char * const path) {
        fFormatContext = avformat_alloc_context();
        if(!fFormatContext) RuntimeThrow("Error allocating AVFormatContext");
        if(avformat_open_input(&fFormatContext, path, nullptr, nullptr) != 0) {
            RuntimeThrow("Could not open file");
        }
        if(avformat_find_stream_info(fFormatContext, nullptr) < 0) {
            RuntimeThrow("Could not retrieve stream info");
        }

        // Find the index of the first audio stream
        fVideoStreamIndex = -1;
        const AVCodecParameters *vidCodecPars = nullptr;
        const AVCodec *vidCodec = nullptr;
        fVideoStream = nullptr;
        for(uint i = 0; i < fFormatContext->nb_streams; i++) {
            const AVStream * const  iStream = fFormatContext->streams[i];
            const AVCodecParameters * const iCodecPars = iStream->codecpar;
            const AVMediaType &iMediaType = iCodecPars->codec_type;
            if(iMediaType == AVMEDIA_TYPE_VIDEO) {
                fVideoStreamIndex = static_cast<int>(i);
                vidCodecPars = iCodecPars;
                vidCodec = avcodec_find_decoder(vidCodecPars->codec_id);
                fVideoStream = fFormatContext->streams[fVideoStreamIndex];
                fTimeBaseDen = fVideoStream->r_frame_rate.den; //avg_frame_rate ??
                fTimeBaseNum = fVideoStream->r_frame_rate.num; //avg_frame_rate ??
                if(fTimeBaseDen == 0)
                    RuntimeThrow("Invalid video frame rate denominator (0)");
                fFps = static_cast<qreal>(fTimeBaseNum)/fTimeBaseDen;
                if(fVideoStream->nb_frames > 0) {
                    fFrameCount = static_cast<int>(fVideoStream->nb_frames);
                } else {
                    const int64_t duration = fFormatContext->duration +
                            (fFormatContext->duration <= INT64_MAX - 5000 ? 5000 : 0);
                    fFrameCount = qFloor(duration*fFps/AV_TIME_BASE);
                }
                break;
            }
        }
        if(fVideoStreamIndex == -1)
            RuntimeThrow("Could not retrieve video stream");
        if(!vidCodec) RuntimeThrow("Unsuported codec");

        fCodecContext = avcodec_alloc_context3(vidCodec);
        if(!fCodecContext) RuntimeThrow("Error allocating AVCodecContext");
        if(avcodec_parameters_to_context(fCodecContext, vidCodecPars) < 0) {
            RuntimeThrow("Failed to copy codec params to codec context");
        }

        if(avcodec_open2(fCodecContext, vidCodec, nullptr) < 0) {
            RuntimeThrow("Failed to open codec");
        }
        fSwsContext =
              sws_getContext(fCodecContext->width, fCodecContext->height,
                             fCodecContext->pix_fmt,
                             fCodecContext->width, fCodecContext->height,
                             AV_PIX_FMT_BGRA, SWS_BICUBIC,
                             nullptr, nullptr, nullptr);

        fPacket = av_packet_alloc();
        if(!fPacket) RuntimeThrow("Error allocating AVPacket");
        fDecodedFrame = av_frame_alloc();
        if(!fDecodedFrame) RuntimeThrow("Error allocating AVFrame");

        fOpened = true;
    }
};
class VideoCacheHandler;
class VideoFrameLoader : public _HDDTask {
    friend class StdSelfRef;
protected:
    VideoFrameLoader(VideoCacheHandler * const cacheHandler,
                     const VideoStreamsData * const openedVideo,
                     const int& frameId) :
        mCacheHandler(cacheHandler), mOpenedVideo(openedVideo),
        mFrameId(frameId) {}

    void afterProcessingFinished();
    void afterCanceled();
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
        const qreal fps = mOpenedVideo->fFps;

        if(mFrameId == 0)
            avformat_seek_file(formatContext, videoStreamIndex,
                               INT64_MIN, 0, INT64_MAX, 0);
        else {
            const int64_t tsms = qRound(mFrameId * 1000 / fps);
            int64_t tm = av_rescale(tsms, videoStream->time_base.den,
                                    videoStream->time_base.num)/1000;
            const int64_t tsms0 = qRound((mFrameId - 2*fps)* 1000 / fps);
            int64_t tm0 = av_rescale(tsms0, videoStream->time_base.den,
                                     videoStream->time_base.num)/1000;
            if(avformat_seek_file(formatContext, videoStreamIndex, tm0,
                                  tm, tm, AVSEEK_FLAG_FRAME) < 0) {
                qDebug() << "Failed to seek to " << mFrameId;
                avformat_seek_file(formatContext, videoStreamIndex,
                                   INT64_MIN, 0, INT64_MAX, 0);
            }
        }

        avcodec_flush_buffers(codecContext);

        int64_t pts = 0;
        bool frameReceived = false;
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
                    if(response == AVERROR_EOF) {
                        return;
                    } else if(response == AVERROR(EAGAIN)) {
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
            }

            // calculate PTS:
            pts = av_frame_get_best_effort_timestamp(decodedFrame);
            pts = av_rescale_q(pts, videoStream->time_base, AV_TIME_BASE_Q);
            const int currFrame = qRound(pts/1000000.*fps);
            if(currFrame >= mFrameId) {
                frameReceived = true;
                break;
            }
            av_frame_unref(decodedFrame);
        }

        if(frameReceived) {
            const auto info = SkiaHelpers::getPremulBGRAInfo(
                        codecContext->width, codecContext->height);
            SkBitmap bitmap;
            bitmap.allocPixels(info);

            SkPixmap pixmap;
            bitmap.peekPixels(&pixmap);

            void * const addr = pixmap.writable_addr();
            uint8_t * const dstSk[] = { static_cast<unsigned char*>(addr) };
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

    VideoCacheHandler * const mCacheHandler;
    const VideoStreamsData * const mOpenedVideo;
    const int mFrameId;
    sk_sp<SkImage> mLoadedFrame;
};

class VideoCacheHandler : public AnimationCacheHandler {
    friend class StdSelfRef;
protected:
    VideoCacheHandler(const QString &filePath);
public:
    sk_sp<SkImage> getFrameAtFrame(const int &relFrame);
    sk_sp<SkImage> getFrameAtOrBeforeFrame(const int &relFrame);
    _ScheduledTask *scheduleFrameLoad(const int &frame);

    void clearCache();
    void replace();

    void frameLoaderFinished(const int &frame,
                             const sk_sp<SkImage>& image);
    void frameLoaderCanceled(const int &frameId) {
        removeFrameLoader(frameId);
    }
protected:
    VideoFrameLoader * getFrameLoader(const int& frame) {
        if(mFrameCount <= 0 || frame >= mFrameCount) return nullptr;
        const int id = mFramesBeingLoaded.indexOf(frame);
        if(id >= 0) return mFrameLoaders.at(id).get();
        return nullptr;
    }

    VideoFrameLoader * addFrameLoader(const int& frame) {
        mFramesBeingLoaded << frame;
        const auto loader = SPtrCreate(VideoFrameLoader)(
                    this, &mVideoStreamsData, frame);
        mFrameLoaders << loader;
        return loader.get();
    }

    void removeFrameLoader(const int& frame) {
        const int id = mFramesBeingLoaded.indexOf(frame);
        mFramesBeingLoaded.removeAt(id);
        mFrameLoaders.removeAt(id);
    }

    void openVideoStream() {
        mVideoStreamsData.open(mFilePath);
        mFrameCount = mVideoStreamsData.fFrameCount;
    }

    QList<int> mFramesBeingLoaded;
    QList<stdsptr<VideoFrameLoader>> mFrameLoaders;

    VideoStreamsData mVideoStreamsData;
    RenderCacheHandler mFramesCache;
};

#endif // VIDEOCACHEHANDLER_H
