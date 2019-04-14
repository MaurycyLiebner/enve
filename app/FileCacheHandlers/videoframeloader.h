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
    void readFrame();

    VideoCacheHandler * const mCacheHandler;
    const VideoStreamsData * const mOpenedVideo;
    const int mFrameId;
    sk_sp<SkImage> mLoadedFrame;
};

#endif // VIDEOFRAMELOADER_H
