#include "soundcachecontainer.h"
#include "soundtmpfilehandlers.h"

SoundCacheContainer::SoundCacheContainer(const FrameRange &range,
                                         HDDCachableCacheHandler * const parent) :
    HDDCachableRangeContainer(range, parent) {}

SoundCacheContainer::SoundCacheContainer(const stdsptr<Samples>& samples,
                                         const FrameRange &range,
                                         HDDCachableCacheHandler * const parent) :
    SoundCacheContainer(range, parent) {
    replaceSamples(samples);
}

stdsptr<_HDDTask> SoundCacheContainer::createTmpFileDataSaver() {
    return SPtrCreate(SoundContainerTmpFileDataSaver)(mSamples, this);
}

stdsptr<_HDDTask> SoundCacheContainer::createTmpFileDataLoader() {
    return SPtrCreate(SoundContainerTmpFileDataLoader)(mTmpFile, this);
}


int SoundCacheContainer::clearMemory() {
    const int bytes = getByteCount();
    mSamples.reset();
    return bytes;
}
