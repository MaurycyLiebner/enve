#ifndef VIDEOCACHEHANDLER_H
#define VIDEOCACHEHANDLER_H
#include "animationcachehandler.h"
#include "videostreamsdata.h"
#include <set>
#include "filecachehandler.h"
class VideoFrameLoader;
class VideoFrameHandler;

class VideoFrameCacheHandler : public FileDataCacheHandler {
    Q_OBJECT
public:
    VideoFrameCacheHandler() {}

    void clearCache();
    void replace();
    void afterSourceChanged();

    const HDDCachableCacheHandler& getCacheHandler() const;

    void addFrameLoader(const int frameId, const stdsptr<VideoFrameLoader>& loader);
    VideoFrameLoader * getFrameLoader(const int frame) const;
    void removeFrameLoader(const int frame);
    void frameLoaderFinished(const int frame, const sk_sp<SkImage>& image);
    Task* scheduleFrameHddCacheLoad(const int frame);
    sk_sp<SkImage> getFrameAtFrame(const int relFrame) const;
    sk_sp<SkImage> getFrameAtOrBeforeFrame(const int relFrame) const;
    int getFrameCount() const;
    void setFrameCount(const int count);
signals:
    void frameCountUpdated(int);
private:
    int mFrameCount = 0;
    QList<VideoFrameHandler*> mFrameHandlers;
    QList<int> mFramesBeingLoaded;
    QList<stdsptr<VideoFrameLoader>> mFrameLoaders;
    HDDCachableCacheHandler mFramesCache;
};

class VideoFrameHandler : public AnimationFrameHandler {
    friend class StdSelfRef;
    friend class VideoFrameLoader;
protected:
    VideoFrameHandler(VideoFrameCacheHandler* const cacheHandler);
public:
    sk_sp<SkImage> getFrameAtFrame(const int relFrame);
    sk_sp<SkImage> getFrameAtOrBeforeFrame(const int relFrame);
    Task *scheduleFrameLoad(const int frame);
    int getFrameCount() const;
    void reload();

    void afterSourceChanged();

    void frameLoaderFinished(const int frame, const sk_sp<SkImage>& image);
    void frameLoaderCanceled(const int frameId);

    VideoFrameCacheHandler* getDataHandler() const;
    const HDDCachableCacheHandler& getCacheHandler() const;
protected:
    VideoFrameLoader * getFrameLoader(const int frame);
    VideoFrameLoader * addFrameLoader(const int frameId);
    VideoFrameLoader * addFrameLoader(const int frameId, AVFrame * const frame);
    void removeFrameLoader(const int frame);

    void openVideoStream();
private:
    std::set<int> mNeededFrames;

    VideoFrameCacheHandler* const mDataHandler;
    stdsptr<VideoStreamsData> mVideoStreamsData;
};

#endif // VIDEOCACHEHANDLER_H
