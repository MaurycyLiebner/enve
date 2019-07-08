#include "animationcachehandler.h"

AnimationFrameHandler::AnimationFrameHandler() {}

sk_sp<SkImage> AnimationFrameHandler::getFrameCopyAtFrame(const int relFrame) {
    const sk_sp<SkImage> imageToCopy = getFrameAtFrame(relFrame);
    return SkiaHelpers::makeCopy(imageToCopy);
}

sk_sp<SkImage> AnimationFrameHandler::getFrameCopyAtOrBeforeFrame(const int relFrame) {
    const sk_sp<SkImage> imageToCopy = getFrameAtOrBeforeFrame(relFrame);
    return SkiaHelpers::makeCopy(imageToCopy);
}
