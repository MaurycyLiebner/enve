#ifndef VIDEOENCODER_H
#define VIDEOENCODER_H
#include <QString>
#include <QList>
#include "skia/skiaincludes.h"
#include "updatable.h"
#include "renderinstancesettings.h"
#include "framerange.h"
#include "CacheHandlers/samples.h"
extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
    #include <libavresample/avresample.h>
    #include <libavutil/channel_layout.h>
    #include <libavutil/mathematics.h>
    #include <libavutil/opt.h>
}
class ImageCacheContainer;


class SoundIterator {
public:
    SoundIterator() {
        float fT = 0;
        const qreal tincr = 2 * M_PI * 110 / 44100;
        float fTincr = static_cast<float>(tincr);
        /* increment frequency by 110 Hz per second */
        const float fTincr2 = static_cast<float>(tincr / 44100);
        for(int i = 0; i < 10; i++) {
            const auto samples = SPtrCreate(Samples)(SampleRange{0, 44099});
            float * q = samples->fData;
            for(int j = 0; j < 44100; j++) {
                *q++ = sin(fT);
                fT += fTincr;
                fTincr += fTincr2;
            }
            mSamples << samples;
        }
        updateCurrent();
    }

    bool hasValue() const {
        return !mSamples.isEmpty();
    }

    bool next() {
        if(mSamples.isEmpty()) return false;
        if(mCurrentSampleId >= mLastSampleId) {
            mSamples.removeFirst();
            if(!updateCurrent()) {
                mCurrentValue = 0;
                return false;
            }
        } else mCurrentSampleId++;
        mCurrentValue = mSamples.first()->fData[mCurrentSampleId];
        return true;
    }

    float value() const {
        return mCurrentValue;
    }
private:
    bool updateCurrent() {
        if(mSamples.isEmpty()) return false;
        const auto& currSamples = mSamples.first();
        mCurrentSampleId = 0;
        mLastSampleId = currSamples->fSampleRange.span() - 1;
        mCurrentValue = *currSamples->fData;
//        mCurrentSample = currSamples->fData;
//        mLastSample = currSamples->fData + currSamples->fSampleRange.span() - 1;
        return true;
    }

    float mCurrentValue = 0;
    int mCurrentSampleId;
    int mLastSampleId;
    QList<stdsptr<Samples>> mSamples;
};

typedef struct OutputStream {
    // pts of the next frame that will be generated
    int64_t fNextPts;
    float fT, fTincr, fTincr2;

    AVStream *fStream = nullptr;
    AVCodecContext *fCodec = nullptr;
    AVFrame *fFrame = nullptr;
    AVFrame *fTmpFrame = nullptr;
    struct SwsContext *fSwsCtx = nullptr;
    AVAudioResampleContext *fAvr = nullptr;
} OutputStream;

class VideoEncoderEmitter : public QObject {
    Q_OBJECT
public:
    VideoEncoderEmitter() {}
signals:
    void encodingStarted();
    void encodingFinished();
    void encodingInterrupted();

    void encodingStartFailed();
    void encodingFailed();
};

//class FrameEncoder : public HDDTask {
//protected:
//    FrameEncoder(const stdsptr<ImageCacheContainer>& frame) {
//        frame->setBlocked(true);
//    }

//    ~FrameEncoder() {
//        mFrame->setBlocked(false);
//    }
//public:
//    void processTask() {

//    }
//private:
//    const stdsptr<ImageCacheContainer> mFrame;
//};

class VideoEncoder : public HDDTask {
    friend class StdSelfRef;
protected:
    VideoEncoder();
public:
    void processTask();
    void beforeProcessing();
    void afterProcessing();

    void startNewEncoding(RenderInstanceSettings * const settings) {
        if(!mCurrentlyEncoding) startEncoding(settings);
    }

    void interruptCurrentEncoding() {
        if(isActive()) mInterruptEncoding = true;
        else interrupEncoding();
    }

    void finishCurrentEncoding() {
        if(!mCurrentlyEncoding) return;
        if(isActive()) mEncodingFinished = true;
        else finishEncodingSuccess();
    }

    void addContainer(const stdsptr<ImageCacheContainer> &cont);

    static VideoEncoder *mVideoEncoderInstance;
    static VideoEncoderEmitter *getVideoEncoderEmitter();

    static void sInterruptEncoding();
    static void sStartEncoding(RenderInstanceSettings *settings);
    static void sAddCacheContainerToEncoder(const stdsptr<ImageCacheContainer> &cont);
    static void sFinishEncoding();
    static bool sEncodingSuccessfulyStarted();

    VideoEncoderEmitter *getEmitter() {
        return &mEmitter;
    }

    const bool& getCurrentlyEncoding() const {
        return mCurrentlyEncoding;
    }
protected:
    void clearContainers();
    VideoEncoderEmitter mEmitter;
    void interrupEncoding();
    void finishEncodingSuccess();
    void finishEncodingNow();
    void startEncoding(RenderInstanceSettings * const settings);
    void startEncodingNow();

    bool mEncodingSuccesfull = false;
    bool mEncodingFinished = false;
    bool mInterruptEncoding = false;

    OutputStream mVideoStream;
    OutputStream mAudioStream;
    AVFormatContext *mFormatContext = nullptr;
    AVOutputFormat *mOutputFormat = nullptr;
    bool mCurrentlyEncoding = false;
    QList<stdsptr<ImageCacheContainer>> mNextContainers;

    RenderSettings mRenderSettings;
    OutputSettings mOutputSettings;
    RenderInstanceSettings *mRenderInstanceSettings = nullptr;
    QByteArray mPathByteArray;
    bool mHaveVideo = false;
    bool mHaveAudio = false;
    bool mEncodeVideo = false;
    bool mEncodeAudio = false;

    int _mCurrentContainerId = 0;
    int _mCurrentContainerFrame = 0; // some containers will add multiple frames
    FrameRange _mRenderRange;

    QList<stdsptr<ImageCacheContainer>> _mContainers;
    SoundIterator mSoundIterator;
};

#endif // VIDEOENCODER_H
