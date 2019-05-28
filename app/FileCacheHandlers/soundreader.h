#ifndef SOUNDREADER_H
#define SOUNDREADER_H
#include "updatable.h"
#include "CacheHandlers/samples.h"
#include "framerange.h"
extern "C" {
    #include <libavutil/opt.h>
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswresample/swresample.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
}

class SoundCacheHandler;
struct AudioStreamsData;

class SoundReader : public HDDTask {
    friend class StdSelfRef;
protected:
    SoundReader(SoundCacheHandler * const cacheHandler,
                const stdsptr<AudioStreamsData>& openedAudio,
                const int& secondId, const SampleRange& sampleRange) :
        mCacheHandler(cacheHandler), mOpenedAudio(openedAudio),
        mSecondId(secondId), mSampleRange(sampleRange) {}

    void afterProcessing();
    void afterCanceled();
public:
    void processTask() {
        readFrame();
    }
protected:
    const stdsptr<Samples>& getSamples() const {
        return mSamples;
    }
private:
    void readFrame();

    SoundCacheHandler * const mCacheHandler;
    const stdsptr<AudioStreamsData> mOpenedAudio;
    const int mSecondId;
    const SampleRange mSampleRange;
    stdsptr<Samples> mSamples;
};

#include "Sound/soundmerger.h"
class SoundReaderForMerger : public SoundReader {
    friend class StdSelfRef;
    struct SingleSound {
        int fSampleShift;
        SampleRange fSamplesRange;
        QrealSnapshot fVolume;
        qreal fSpeed;
    };
protected:
    SoundReaderForMerger(SoundCacheHandler * const cacheHandler,
                         const stdsptr<AudioStreamsData>& openedAudio,
                         const int& secondId, const SampleRange& sampleRange) :
        SoundReader(cacheHandler, openedAudio, secondId, sampleRange) {}

    void afterProcessing();
public:
    void addSingleSound(const int& sampleShift,
                        const SampleRange& absRange,
                        const QrealSnapshot& volume,
                        const qreal& speed) {
        mSSAbsRanges.append({sampleShift, absRange, volume, speed});
    }

    void addMerger(SoundMerger * const merger) {
        mMergers << merger;
    }
private:
    QList<SingleSound> mSSAbsRanges;
    QList<stdptr<SoundMerger>> mMergers;
};
#endif // SOUNDREADER_H
