#ifndef ANIMATIONCACHEHANDLER_H
#define ANIMATIONCACHEHANDLER_H
#include "smartPointers/stdselfref.h"
#include "skia/skiahelpers.h"
class Task;

class AnimationFrameHandler : public StdSelfRef {
protected:
    AnimationFrameHandler();
public:
    virtual sk_sp<SkImage> getFrameAtFrame(const int relFrame) = 0;
    virtual sk_sp<SkImage> getFrameAtOrBeforeFrame(const int relFrame) = 0;
    virtual Task* scheduleFrameLoad(const int frame) = 0;
    virtual int getFrameCount() const = 0;
    virtual void reload() = 0;

    sk_sp<SkImage> getFrameCopyAtFrame(const int relFrame);
    sk_sp<SkImage> getFrameCopyAtOrBeforeFrame(const int relFrame);
protected:
};
#endif // ANIMATIONCACHEHANDLER_H
