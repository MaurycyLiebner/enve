#ifndef IMAGESEQUENCECACHEHANDLER_H
#define IMAGESEQUENCECACHEHANDLER_H
#include "imagecachehandler.h"
#include "animationcachehandler.h"

class ImageSequenceCacheHandler : public AnimationCacheHandler {
    friend class StdSelfRef;
public:
    sk_sp<SkImage> getFrameAtFrame(const int relFrame);
    sk_sp<SkImage> getFrameAtOrBeforeFrame(const int relFrame);

    void updateFrameCount();

    void processTask() {}

    void clearCache();

    Task* scheduleFrameLoad(const int frame);
protected:
    ImageSequenceCacheHandler(const QStringList &framePaths);

    QStringList mFramePaths;
    QList<stdptr<ImageCacheHandler>> mFrameImageHandlers;
};
#endif // IMAGESEQUENCECACHEHANDLER_H
