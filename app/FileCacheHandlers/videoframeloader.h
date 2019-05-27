#ifndef VIDEOFRAMELOADER_H
#define VIDEOFRAMELOADER_H
#include "updatable.h"
#include "skia/skiaincludes.h"
extern "C" {
    #include <libavutil/opt.h>
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswresample/swresample.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}

class VideoCacheHandler;
struct VideoStreamsData;
class VideoFrameLoader : public HDDTask {
    friend class StdSelfRef;
protected:
    VideoFrameLoader(VideoCacheHandler * const cacheHandler,
                     const stdsptr<VideoStreamsData>& openedVideo,
                     const int& frameId) :
        mCacheHandler(cacheHandler), mOpenedVideo(openedVideo),
        mFrameId(frameId) {}

    void afterProcessing();
    void afterCanceled();

    void installFrame(AVFrame * const src,
                      AVCodecContext * const codecContext) {
        mFrameToConvert = av_frame_alloc();
        av_frame_move_ref(mFrameToConvert, src);
        av_frame_unref(src);

        mSwsContext = sws_getContext(codecContext->width,
                                     codecContext->height,
                                     codecContext->pix_fmt,
                                     codecContext->width,
                                     codecContext->height,
                                     AV_PIX_FMT_BGRA, SWS_BICUBIC,
                                     nullptr, nullptr, nullptr);
    }
public:
    void processTask() { readFrame(); }
private:
    void setupSwsContext(AVCodecContext * const codecContext);
    void readFrame();
    void convertFrame();

    VideoCacheHandler * const mCacheHandler;
    const stdsptr<VideoStreamsData> mOpenedVideo;
    const int mFrameId;
    sk_sp<SkImage> mLoadedFrame;

    AVFrame * mFrameToConvert = nullptr;
    struct SwsContext * mSwsContext = nullptr;
};

#endif // VIDEOFRAMELOADER_H
