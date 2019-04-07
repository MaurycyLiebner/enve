#ifndef ANIMATIONCACHEHANDLER_H
#define ANIMATIONCACHEHANDLER_H
#include "skia/skiahelpers.h"
#include "filecachehandler.h"

class AnimationCacheHandler : public FileCacheHandler {
protected:
    AnimationCacheHandler(const QString &filePath);
    AnimationCacheHandler();
public:
    virtual sk_sp<SkImage> getFrameAtFrame(const int &relFrame) = 0;
    virtual sk_sp<SkImage> getFrameAtOrBeforeFrame(const int &relFrame) = 0;
    virtual _ScheduledTask* scheduleFrameLoad(const int &frame) = 0;

    sk_sp<SkImage> getFrameCopyAtFrame(const int &relFrame) {
        const sk_sp<SkImage> imageToCopy = getFrameAtFrame(relFrame);
        return SkiaHelpers::makeSkImageCopy(imageToCopy);
    }

    sk_sp<SkImage> getFrameCopyAtOrBeforeFrame(const int &relFrame) {
        const sk_sp<SkImage> imageToCopy = getFrameAtOrBeforeFrame(relFrame);
        return SkiaHelpers::makeSkImageCopy(imageToCopy);
    }

    int getFrameCount() { return mFrameCount; }

protected:
    int mFrameCount = 0;
};
#endif // ANIMATIONCACHEHANDLER_H
