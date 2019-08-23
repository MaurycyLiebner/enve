#include "soundcachecontainer.h"
#include "soundtmpfilehandlers.h"

SoundCacheContainer::SoundCacheContainer(const iValueRange &second,
                                         HddCachableCacheHandler * const parent) :
    HddCachableRangeCont(second, parent) {}

SoundCacheContainer::SoundCacheContainer(const stdsptr<Samples>& samples,
                                         const iValueRange &second,
                                         HddCachableCacheHandler * const parent) :
    SoundCacheContainer(second, parent) {
    replaceSamples(samples);
}

stdsptr<eHddTask> SoundCacheContainer::createTmpFileDataSaver() {
    return enve::make_shared<SoundContainerTmpFileDataSaver>(mSamples, this);
}

stdsptr<eHddTask> SoundCacheContainer::createTmpFileDataLoader() {
    return enve::make_shared<SoundContainerTmpFileDataLoader>(mTmpFile, this);
}

int SoundCacheContainer::clearMemory() {
    const int bytes = getByteCount();
    mSamples.reset();
    return bytes;
}
