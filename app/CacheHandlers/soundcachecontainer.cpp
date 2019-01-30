#include "soundcachecontainer.h"
#include "soundtmpfilehandlers.h"

SoundCacheContainer::SoundCacheContainer(const FrameRange &range,
                                         Handler * const parent) :
    Base(range, parent) {}

SoundCacheContainer::SoundCacheContainer(const stdsptr<Samples>& samples,
                                         const FrameRange &range,
                                         Handler * const parent) :
    SoundCacheContainer(range, parent) {
    replaceSamples(samples);
}

stdsptr<_HDDTask> SoundCacheContainer::createTmpFileDataSaver() {
    return SPtrCreate(SoundContainerTmpFileDataSaver)(mSamples, this);
}

stdsptr<_HDDTask> SoundCacheContainer::createTmpFileDataLoader() {
    return SPtrCreate(SoundContainerTmpFileDataLoader)(mTmpFile, this);
}


void SoundCacheContainer::clearDataAfterSaved() {
    mSamples.reset();
}
