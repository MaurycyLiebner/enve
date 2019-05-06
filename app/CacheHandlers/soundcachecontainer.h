#ifndef SOUNDCACHECONTAINER_H
#define SOUNDCACHECONTAINER_H
#include "hddcachablerangecontainer.h"

struct Samples : public StdSelfRef {
    friend class StdSelfRef;
protected:
    Samples() : fData(nullptr), fSamplesCount(0) {}

    Samples(const int& size) :
        fData(static_cast<float*>(malloc(static_cast<size_t>(size)*sizeof(float)))),
        fSamplesCount(size) {}

    Samples(float * const data, const int& size) :
        fData(data), fSamplesCount(size) {}
public:
    ~Samples() { free(fData); }
    float * const fData;
    const int fSamplesCount;
};

class SoundCacheContainer : public HDDCachableRangeContainer {
    friend class StdSelfRef;
    typedef stdsptr<SoundCacheContainer> stdptrSCC;
protected:
    SoundCacheContainer(const int &frame,
                        HDDCachableCacheHandler * const parent);
    SoundCacheContainer(const stdsptr<Samples>& samples,
                        const int &frame,
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
