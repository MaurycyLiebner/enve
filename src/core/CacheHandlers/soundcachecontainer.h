#ifndef SOUNDCACHECONTAINER_H
#define SOUNDCACHECONTAINER_H
#include "hddcachablerangecont.h"
#include "CacheHandlers/samples.h"

class SoundCacheContainer : public HDDCachableRangeCont {
    e_OBJECT
    typedef stdsptr<SoundCacheContainer> stdptrSCC;
protected:
    SoundCacheContainer(const iValueRange &second,
                        HDDCachableCacheHandler * const parent);
    SoundCacheContainer(const stdsptr<Samples>& samples,
                        const iValueRange &second,
                        HDDCachableCacheHandler * const parent);
public:
    int getByteCount() {
        if(!mSamples) return 0;
        return mSamples->fSampleRange.span()*static_cast<int>(sizeof(float));
    }

    stdsptr<Samples> getSamples() { return mSamples; }

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
