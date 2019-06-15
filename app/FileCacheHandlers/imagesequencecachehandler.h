#ifndef IMAGESEQUENCECACHEHANDLER_H
#define IMAGESEQUENCECACHEHANDLER_H
#include "imagecachehandler.h"
#include "animationcachehandler.h"

class ImageSequenceCacheHandler : public AnimationCacheHandler {
    friend class StdSelfRef;
protected:
    ImageSequenceCacheHandler(const QStringList &framePaths);
public:
    sk_sp<SkImage> getFrameAtFrame(const int relFrame);
    sk_sp<SkImage> getFrameAtOrBeforeFrame(const int relFrame);
    void clearCache();
    Task* scheduleFrameLoad(const int frame);

    void updateFrameCount();
private:
    QStringList mFramePaths;
    QList<ImageCacheHandler*> mFrameImageHandlers;
};
#endif // IMAGESEQUENCECACHEHANDLER_H
