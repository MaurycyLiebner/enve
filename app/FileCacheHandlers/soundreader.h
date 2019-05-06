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
                AudioStreamsData * const openedAudio,
                const int& secondId, const SampleRange& sampleRange) :
        mCacheHandler(cacheHandler), mOpenedAudio(openedAudio),
        mSecondId(secondId), mSampleRange(sampleRange) {}

    void afterProcessing();

    void afterCanceled();

public:
    void processTask() {
        readFrame();
    }
private:
    void readFrame();

    SoundCacheHandler * const mCacheHandler;
    const AudioStreamsData * const mOpenedAudio;
    const int mSecondId;
    const SampleRange mSampleRange;
    stdsptr<Samples> mSamples;
};
#endif // SOUNDREADER_H
