#include "soundcachecontainer.h"
#include "soundtmpfilehandlers.h"

SoundCacheContainer::SoundCacheContainer(const iValueRange &second,
                                         HDDCachableCacheHandler * const parent) :
    HDDCachableRangeCont(second, parent) {}

SoundCacheContainer::SoundCacheContainer(const stdsptr<Samples>& samples,
                                         const iValueRange &second,
                                         HDDCachableCacheHandler * const parent) :
    SoundCacheContainer(second, parent) {
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
