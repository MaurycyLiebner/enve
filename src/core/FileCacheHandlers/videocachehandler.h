#ifndef VIDEOCACHEHANDLER_H
#define VIDEOCACHEHANDLER_H
#include "animationcachehandler.h"
#include "videostreamsdata.h"
#include <set>
#include "filecachehandler.h"
class VideoFrameLoader;
class VideoFrameHandler;

class VideoDataHandler : public FileDataCacheHandler {
    Q_OBJECT
public:
    VideoDataHandler() {}

    void clearCache();
    void afterSourceChanged();

    const HddCachableCacheHandler& getCacheHandler() const;

    void addFrameLoader(const int frameId, const stdsptr<VideoFrameLoader>& loader);
    VideoFrameLoader * getFrameLoader(const int frame) const;
    void removeFrameLoader(const int frame);
    void frameLoaderFinished(const int frame, const sk_sp<SkImage>& image);
    eTask* scheduleFrameHddCacheLoad(const int frame);
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
    HddCachableCacheHandler mFramesCache;
};

class VideoFrameHandler : public AnimationFrameHandler {
    e_OBJECT
    friend class VideoFrameLoader;
protected:
    VideoFrameHandler(VideoDataHandler* const cacheHandler);
public:
    sk_sp<SkImage> getFrameAtFrame(const int relFrame);
    sk_sp<SkImage> getFrameAtOrBeforeFrame(const int relFrame);
    eTask *scheduleFrameLoad(const int frame);
    int getFrameCount() const;
    void reload();

    void afterSourceChanged();

    void frameLoaderFinished(const int frame, const sk_sp<SkImage>& image);
    void frameLoaderCanceled(const int frameId);

    VideoDataHandler* getDataHandler() const;
    const HddCachableCacheHandler& getCacheHandler() const;
protected:
    VideoFrameLoader * getFrameLoader(const int frame);
    VideoFrameLoader * addFrameLoader(const int frameId);
    VideoFrameLoader * addFrameLoader(const int frameId, AVFrame * const frame);
    void removeFrameLoader(const int frame);

    void openVideoStream();
private:
    std::set<int> mNeededFrames;

    VideoDataHandler* const mDataHandler;
    stdsptr<VideoStreamsData> mVideoStreamsData;
};
#include "CacheHandlers/soundcachehandler.h"
class VideoFileHandler : public FileCacheHandler {
    e_OBJECT
protected:
    VideoFileHandler() {}
    void afterPathSet(const QString& path);

    void reload() {
        mDataHandler->reload();
        mSoundHandler->reload();
    }
public:
    void replace(QWidget* const parent);

    VideoDataHandler* getFrameHandler() const {
        return mDataHandler.get();
    }

    SoundDataHandler* getSoundHandler() const {
        return mSoundHandler.get();
    }
private:
    qsptr<VideoDataHandler> mDataHandler;
    qsptr<SoundDataHandler> mSoundHandler;
};

#endif // VIDEOCACHEHANDLER_H
