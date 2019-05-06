#include "soundcachecontainer.h"
#include "soundtmpfilehandlers.h"

SoundCacheContainer::SoundCacheContainer(const int &frame,
                                         HDDCachableCacheHandler * const parent) :
    HDDCachableRangeContainer({frame, frame}, parent) {}

SoundCacheContainer::SoundCacheContainer(const stdsptr<Samples>& samples,
                                         const int &frame,
                                         HDDCachableCacheHandler * const parent) :
    SoundCacheContainer(frame, parent) {
    replaceSamples(samples);
}

stdsptr<HDDTask> SoundCacheContainer::createTmpFileDataSaver() {
    return SPtrCreate(SoundContainerTmpFileDataSaver)(mSamples, this);
}

stdsptr<HDDTask> SoundCacheContainer::createTmpFileDataLoader() {
    return SPtrCreate(SoundContainerTmpFileDataLoader)(mTmpFile, this);
}

int SoundCacheContainer::clearMemory() {
    const int bytes = getByteCount();
    mSamples.reset();
    return bytes;
}
