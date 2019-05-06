#include "soundcachehandler.h"
#include "FileCacheHandlers/soundreader.h"
#include "Sound/singlesound.h"

stdsptr<Samples> SoundCacheHandler::getSamplesForSecond(const int &secondId) {
    const auto cont = mSecondsCache.atRelFrame
            <SoundCacheContainer>(secondId);
    if(!cont) return nullptr;
    return cont->getSamples();
}

void SoundCacheHandler::secondLoaderFinished(
        const int& secondId,
        const stdsptr<Samples>& samples) {
    if(samples) {
        mSecondsCache.createNew<SoundCacheContainer>(secondId, samples);
    }
    removeSecondLoader(secondId);
}

Task *SoundCacheHandler::scheduleSecondLoad(const int &secondId) {
    if(secondId < 0 || secondId >= mSecondCount)
        RuntimeThrow("Second outside of range " + std::to_string(secondId));
    const auto currLoader = getSecondLoader(secondId);
    if(currLoader) return currLoader;
    const auto contAtFrame = mSecondsCache.atRelFrame
            <SoundCacheContainer>(secondId);
    if(contAtFrame) return contAtFrame->scheduleLoadFromTmpFile();
    const auto loader = addSecondLoader(secondId);
    loader->scheduleTask();
    return loader;
}

void SoundCacheHandler::loadSamples(const int &secondId) {
    const int sR = mSingleSound->getSampleRate();
    const SampleRange& range = {secondId*sR, (secondId + 1)*sR - 1};
    const auto reader = SPtrCreate(SoundReader)(this, &mAudioStreamsData,
                                                secondId, range);
    reader->scheduleTask();
}

SoundReader *SoundCacheHandler::addSecondLoader(const int &secondId) {
    if(mSecondsBeingLoaded.contains(secondId) ||
            getSamplesForSecond(secondId))
        RuntimeThrow("Trying to unnecessarily reload video frame");
    mSecondsBeingLoaded << secondId;
    const int sR = mSingleSound->getSampleRate();
    const SampleRange& range = {secondId*sR, (secondId + 1)*sR - 1};
    const auto reader = SPtrCreate(SoundReader)(this, &mAudioStreamsData,
                                                secondId, range);
    mSecondLoaders << reader;
    return reader.get();
}
