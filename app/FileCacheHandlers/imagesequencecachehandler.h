#ifndef IMAGESEQUENCECACHEHANDLER_H
#define IMAGESEQUENCECACHEHANDLER_H
#include "imagecachehandler.h"
#include "animationcachehandler.h"

class ImageSequenceCacheHandler : public AnimationFrameHandler {
    friend class StdSelfRef;
protected:
    ImageSequenceCacheHandler(const QStringList &framePaths);
public:
    sk_sp<SkImage> getFrameAtFrame(const int relFrame);
    sk_sp<SkImage> getFrameAtOrBeforeFrame(const int relFrame);
    Task* scheduleFrameLoad(const int frame);
    void reload();
    int getFrameCount() const { return mFramePaths.count(); }
private:
    QStringList mFramePaths;
    QList<ImageCacheHandler*> mFrameImageHandlers;
};
#endif // IMAGESEQUENCECACHEHANDLER_H
