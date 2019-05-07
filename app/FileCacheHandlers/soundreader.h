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
                const stdsptr<const AudioStreamsData>& openedAudio,
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
    const stdsptr<const AudioStreamsData> mOpenedAudio;
    const int mSecondId;
    const SampleRange mSampleRange;
    stdsptr<Samples> mSamples;
};

#include "Sound/soundmerger.h"
class SoundReaderForMerger : public SoundReader {
    friend class StdSelfRef;
protected:
    SoundReaderForMerger(SoundCacheHandler * const cacheHandler,
                         const stdsptr<const AudioStreamsData>& openedAudio,
                         const int& secondId, const SampleRange& sampleRange,
                         SoundMerger * const merger) :
        SoundReader(cacheHandler, openedAudio, secondId, sampleRange),
        mMerger(merger) {}
public:
    void afterProcessingAsContainerStep() final;

    void addSSAbsRange(const SampleRange& sampleRange) {
        mSSAbsRanges << sampleRange;
    }
private:
    QList<SampleRange> mSSAbsRanges;
    const stdptr<SoundMerger> mMerger;
};
#endif // SOUNDREADER_H
