#ifndef ANIMATIONCACHEHANDLER_H
#define ANIMATIONCACHEHANDLER_H
#include "skia/skimagecopy.h"
#include "filecachehandler.h"

class AnimationCacheHandler : public FileCacheHandler {
public:
    virtual sk_sp<SkImage> getFrameAtFrame(const int &relFrame) = 0;
    virtual sk_sp<SkImage> getFrameAtOrBeforeFrame(const int &relFrame) = 0;
    sk_sp<SkImage> getFrameCopyAtFrame(const int &relFrame) {
        sk_sp<SkImage> imageToCopy = getFrameAtFrame(relFrame);
        return makeSkImageCopy(imageToCopy);
    }

    sk_sp<SkImage> getFrameCopyAtOrBeforeFrame(const int &relFrame) {
        sk_sp<SkImage> imageToCopy = getFrameAtOrBeforeFrame(relFrame);
        return makeSkImageCopy(imageToCopy);
    }

    virtual _ScheduledTask* scheduleFrameLoad(const int &frame) = 0;
    const int &getFramesCount() { return mFramesCount; }
protected:
    AnimationCacheHandler(const QString &filePath);
    AnimationCacheHandler();

    int mFramesCount = 0;
    virtual void updateFrameCount() = 0;
};
#endif // ANIMATIONCACHEHANDLER_H
