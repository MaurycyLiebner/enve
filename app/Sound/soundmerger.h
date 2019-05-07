#ifndef SOUNDMERGER_H
#define SOUNDMERGER_H
#include "updatable.h"
#include "CacheHandlers/samples.h"
#include "soundcomposition.h"

struct SingleSoundData {
    int fSampleShift;
    SampleRange fSSAbsRange;
    stdsptr<Samples> fSamples;
};

class SoundMerger : public ContainerTask {
    friend class StdSelfRef;
protected:
    SoundMerger(const int& secondId, const SampleRange& sampleRange,
                SoundComposition* const composition) :
        mSecondId(secondId), mSampleRange(sampleRange),
        mComposition(composition) {

    }

    void afterProcessing() {
        ContainerTask::afterProcessing();
        if(!mComposition) return;
        mComposition->secondFinished(mSecondId, mSamples);
    }
public:
    void processTask() {
        mSamples = SPtrCreate(Samples)(mSampleRange);
        const auto& dst = mSamples->fData;
        memset(dst, 0, mSamples->fSampleRange.span()*sizeof(float));
        for(const auto& sound : mSounds) {
            const auto& srcSamples = sound.fSamples;
            const int srcAbsShift = sound.fSampleShift;
            const SampleRange srcAbsRange =
                    srcSamples->fSampleRange.shifted(srcAbsShift)*sound.fSSAbsRange;
            const SampleRange srcNeededAbsRange =
                    srcAbsRange*mSampleRange;
            const SampleRange srcNeededRelRange =
                    srcNeededAbsRange.shifted(-srcSamples->fSampleRange.fMin - srcAbsShift);

            const SampleRange dstAbsRange = srcNeededAbsRange;
            const SampleRange dstRelRange = dstAbsRange.shifted(-mSampleRange.fMin);

            const int nSamples = srcNeededRelRange.span(); // == dstRelRange.span()
            float * const & src = srcSamples->fData;
            for(int i = 0; i < nSamples; i++) {
                const int dstId = dstRelRange.fMin + i;
                const int srcId = srcNeededRelRange.fMin + i;
                dst[dstId] += src[srcId];
            }
        }
    }

    void addSoundToMerge(const SingleSoundData& data) {
        mSounds << data;
    }
private:
    const int mSecondId;
    const SampleRange mSampleRange;
    stdsptr<Samples> mSamples;
    QList<SingleSoundData> mSounds;
    const qptr<SoundComposition> mComposition;
};

#endif // SOUNDMERGER_H
