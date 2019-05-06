#ifndef SOUNDCACHECONTAINER_H
#define SOUNDCACHECONTAINER_H
#include "hddcachablerangecontainer.h"
#include "CacheHandlers/samples.h"

class SoundCacheContainer : public HDDCachableRangeContainer {
    friend class StdSelfRef;
    typedef stdsptr<SoundCacheContainer> stdptrSCC;
protected:
    SoundCacheContainer(const iValueRange &frame,
                        HDDCachableCacheHandler * const parent);
    SoundCacheContainer(const stdsptr<Samples>& samples,
                        const iValueRange &frame,
                        HDDCachableCacheHandler * const parent);
public:
    int getByteCount() {
        if(!mSamples) return 0;
        return mSamples->fSamplesCount*static_cast<int>(sizeof(float));
    }

    stdsptr<Samples> getSamples() { return mSamples; }

    float * getSamplesData() {
        if(!mSamples) return nullptr;
        return mSamples->fData;
    }

    void setDataLoadedFromTmpFile(const stdsptr<Samples> &samples) {
        replaceSamples(samples);
        afterDataLoadedFromTmpFile();
    }

    void replaceSamples(const stdsptr<Samples> &samples) {
        mSamples = samples;
        afterDataReplaced();
    }
protected:
    stdsptr<HDDTask> createTmpFileDataSaver();
    stdsptr<HDDTask> createTmpFileDataLoader();
    int clearMemory();

    stdsptr<Samples> mSamples;
};

#endif // SOUNDCACHECONTAINER_H
