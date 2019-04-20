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
    SoundCacheContainer(const FrameRange &range,
                        HDDCachableCacheHandler * const parent);
    SoundCacheContainer(const stdsptr<Samples>& samples,
                        const FrameRange &range,
                        HDDCachableCacheHandler * const parent);
public:
    int getByteCount() {
        if(!mSamples) return 0;
        return getRange().span()*static_cast<int>(sizeof(float));
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

    //! @brief Provided two containers have to be neighbours.
    //! Assumes all data is stored in memory (not cached to HDD)
    static stdptrSCC sCreateMerge(
            const stdptrSCC& a,
            const stdptrSCC& b,
            HDDCachableCacheHandler * const parent) {
        const bool aFirst = a->getRange() < b->getRange();
        const auto& first = aFirst ? a : b;
        const auto& second = aFirst ? b : a;

        QList<stdptrSCC> list{first, second};
        return sCreateMerge(list, parent);
    }

    //! @brief Provided list has to be an array of neighbours.
    //! Assumes all data is stored in memory (not cached to HDD)
    static stdptrSCC sCreateMerge(const QList<stdptrSCC>& toMerge,
                                  HDDCachableCacheHandler * const parent) {
        if(toMerge.isEmpty()) return nullptr;
        SampleRange lastRange = toMerge.first()->getRange();
        Q_ASSERT(toMerge.first()->storesDataInMemory());
        SampleRange newRange = lastRange;
        for(int i = 1; i < toMerge.count(); i++) {
            const stdptrSCC& iter = toMerge.at(i);
            auto range = iter->getRange();
            Q_ASSERT(range.neighbours(lastRange) &&
                     iter->storesDataInMemory());
            newRange += range;
            lastRange = range;
        }

        auto samples = SPtrCreate(Samples)(newRange.span());
        float * const newData = samples->fData;

        float * dst = newData;
        for(const stdptrSCC& iter : toMerge) {
            const int span = iter->getRange().span();
            const ulong size = static_cast<ulong>(span)*sizeof(float);
            memcpy(dst, iter->getSamplesData(), size);
            dst += span;
        }

        return SPtrCreate(SoundCacheContainer)(samples, newRange,
                                               parent);
    }
protected:
    stdsptr<_HDDTask> createTmpFileDataSaver();
    stdsptr<_HDDTask> createTmpFileDataLoader();
    int clearMemory();

    stdsptr<Samples> mSamples;
};

#endif // SOUNDCACHECONTAINER_H
