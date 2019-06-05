#include "soundcachehandler.h"
#include "FileCacheHandlers/soundreader.h"
#include "Sound/singlesound.h"

stdsptr<Samples> SoundCacheHandler::getSamplesForSecond(const int secondId) {
    const auto cont = mSecondsCache.atRelFrame
            <SoundCacheContainer>(secondId);
    if(!cont) return nullptr;
    return cont->getSamples();
}

void SoundCacheHandler::secondReaderFinished(
        const int secondId,
        const stdsptr<Samples>& samples) {
    if(samples) {
        mSecondsCache.createNew<SoundCacheContainer>(secondId, samples);
    }
    removeSecondReader(secondId);
}

SoundReaderForMerger *SoundCacheHandler::addSecondReader(
        const int secondId) {
    if(mSecondsBeingRead.contains(secondId) ||
       getSamplesForSecond(secondId))
        RuntimeThrow("Trying to unnecessarily reload video frame");
    mSecondsBeingRead << secondId;
    const int sR = 44100;
    const SampleRange& range = {secondId*sR, (secondId + 1)*sR - 1};
    const auto reader = SPtrCreate(SoundReaderForMerger)(
                this, mAudioStreamsData, secondId, range);
    mSecondReaders << reader;
    reader->scheduleTask();
    return reader.get();
}
