#ifndef VIDEOCACHEHANDLER_H
#define VIDEOCACHEHANDLER_H
#include "animationcachehandler.h"
#include "Boxes/rendercachehandler.h"

class VideoCacheHandler : public AnimationCacheHandler {
    friend class StdSelfRef;
public:
    sk_sp<SkImage> getFrameAtFrame(const int &relFrame);
    sk_sp<SkImage> getFrameAtOrBeforeFrame(const int &relFrame);

    void beforeProcessingStarted();

    void _processUpdate();

    void afterProcessingFinished();

    void clearCache();

    void replace();

    const qreal &getFps();

    virtual _ScheduledTask *scheduleFrameLoad(const int &frame);
protected:
    VideoCacheHandler(const QString &filePath);

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

    void updateFrameCount();

    RenderCacheHandler mFramesCache;
};

#endif // VIDEOCACHEHANDLER_H
