#ifndef SOUNDCACHEHANDLER_H
#define SOUNDCACHEHANDLER_H
#include "CacheHandlers/hddcachablecachehandler.h"
#include "CacheHandlers/soundcachecontainer.h"
#include "Decode/audiodecode.h"
class SoundCacheHandler;

class SoundReader : public HDDTask {
    friend class StdSelfRef;
protected:
    SoundReader(SoundCacheHandler * const cacheHandler,
                AudioStreamsData * const openedAudio,
                const int& secondId, const SampleRange& sampleRange) :
        mCacheHandler(cacheHandler), mOpenedAudio(openedAudio),
        mSecondId(secondId), mSampleRange(sampleRange) {}

    void afterProcessing() {
        mCacheHandler->createNew<SoundCacheContainer>(mSamples, mSecondId);
    }

    void afterCanceled() {
        mCacheHandler->frameLoaderCanceled(mSecondId);
    }

public:
    void processTask() {
        readFrame();
    }
private:
    void readFrame() {
        float * data = nullptr;
        gDecodeSoundDataRange(mFilePath.toLatin1().data(), mSampleRange, data);
        mSamples = SPtrCreate(Samples)(data, mSampleRange.span());
    }

    SoundCacheHandler * const mCacheHandler;
    const AudioStreamsData * const mOpenedAudio;
    const int mSecondId;
    const SampleRange mSampleRange;
    stdsptr<Samples> mSamples;
};

class SoundCacheHandler : public HDDCachableCacheHandler {
    typedef stdsptr<SoundCacheContainer> stdptrSCC;
public:

protected:
    void loadSamples(const int& secondId) {
        const int sR = mSingleSound->getSampleRate();
        const SampleRange& range = {secondId*sR, (secondId + 1)*sR - 1};
        const auto reader = SPtrCreate(SoundReader)(this, mAudioStreamsData,
                                                    secondId, range);
        reader->scheduleTask();
    }

    const QString mFilePath;

private:
    const SingleSound * const mSingleSound;
};

#endif // SOUNDCACHEHANDLER_H
