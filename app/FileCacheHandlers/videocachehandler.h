#ifndef VIDEOCACHEHANDLER_H
#define VIDEOCACHEHANDLER_H
#include "animationcachehandler.h"
#include "Boxes/rendercachehandler.h"
#include "videostreamsdata.h"

class VideoCacheHandler : public AnimationCacheHandler {
    friend class StdSelfRef;
protected:
    VideoCacheHandler() {}
public:
    sk_sp<SkImage> getFrameAtFrame(const int &relFrame);
    sk_sp<SkImage> getFrameAtOrBeforeFrame(const int &relFrame);
    Task *scheduleFrameLoad(const int &frame);

    void clearCache();
    void replace();

    void setFilePath(const QString& path) { // throw
        clearCache();
        AnimationCacheHandler::setFilePath(path);
        openVideoStream();
    }

    void frameLoaderFinished(const int &frame,
                             const sk_sp<SkImage>& image);
    void frameLoaderCanceled(const int &frameId) {
        removeFrameLoader(frameId);
    }

    const RenderCacheHandler& getCacheHandler() const {
        return mFramesCache;
    }
protected:
    VideoFrameLoader * getFrameLoader(const int& frame) {
        const int id = mFramesBeingLoaded.indexOf(frame);
        if(id >= 0) return mFrameLoaders.at(id).get();
        return nullptr;
    }

    VideoFrameLoader * addFrameLoader(const int& frame) {
        if(mFramesBeingLoaded.contains(frame) ||
           getFrameAtFrame(frame))
            RuntimeThrow("Trying to unnecessarily reload video frame");
        mFramesBeingLoaded << frame;
        const auto loader = SPtrCreate(VideoFrameLoader)(
                    this, mVideoStreamsData, frame);
        mFrameLoaders << loader;
        return loader.get();
    }

    void removeFrameLoader(const int& frame) {
        const int id = mFramesBeingLoaded.indexOf(frame);
        mFramesBeingLoaded.removeAt(id);
        mFrameLoaders.removeAt(id);
    }

    void openVideoStream() {
        mVideoStreamsData = VideoStreamsData::sOpen(mFilePath);
        mFrameCount = mVideoStreamsData->fFrameCount;
    }
private:
    QList<int> mFramesBeingLoaded;
    QList<stdsptr<VideoFrameLoader>> mFrameLoaders;

    stdsptr<const VideoStreamsData> mVideoStreamsData;
    RenderCacheHandler mFramesCache;
};

#endif // VIDEOCACHEHANDLER_H
