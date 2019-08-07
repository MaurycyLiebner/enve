#ifndef SOUNDMERGER_H
#define SOUNDMERGER_H
#include "Tasks/updatable.h"
#include "CacheHandlers/samples.h"
#include "soundcomposition.h"
#include "Animators/qrealanimator.h"
extern "C" {
    #include <libavutil/opt.h>
    #include <libswresample/swresample.h>
}
struct SingleSoundData {
    int fSampleShift;
    SampleRange fSSAbsRange;
    QrealSnapshot fVolume;
    qreal fStretch;
    stdsptr<Samples> fSamples;
};

class SoundMerger : public CPUTask {
    e_OBJECT
protected:
    SoundMerger(const int secondId, const SampleRange& sampleRange,
                SoundComposition* const composition) :
        mSecondId(secondId), mSampleRange(sampleRange),
        mComposition(composition) {

    }

    void afterProcessing() {
        if(mComposition)
            mComposition->secondFinished(mSecondId, mSamples);
    }
public:
    void process();

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
