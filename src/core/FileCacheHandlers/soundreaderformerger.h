#ifndef SOUNDREADERFORMERGER_H
#define SOUNDREADERFORMERGER_H

#include "soundreader.h"

#include "Animators/qrealsnapshot.h"

class SoundMerger;

class CORE_EXPORT SoundReaderForMerger : public SoundReader {
    e_OBJECT
    struct SingleSound {
        int fSampleShift;
        SampleRange fSamplesRange;
        QrealSnapshot fVolume;
        qreal fSpeed;
    };
protected:
    SoundReaderForMerger(SoundHandler * const cacheHandler,
                         const stdsptr<AudioStreamsData>& openedAudio,
                         const int secondId, const SampleRange& sampleRange);

    void afterProcessing();
public:
    void addSingleSound(void* const soundPtr,
                        const int sampleShift,
                        const SampleRange& absRange,
                        const QrealSnapshot& volume,
                        const qreal speed);

    void addMerger(SoundMerger * const merger);
private:
    QList<void*> mSoundPtrs;
    QList<SingleSound> mSSAbsRanges;
    QList<stdptr<SoundMerger>> mMergers;
};

#endif // SOUNDREADERFORMERGER_H
