#ifndef VIDEOENCODER_H
#define VIDEOENCODER_H
#include <QString>
#include <QList>
#include "skia/skiaincludes.h"
#include "Tasks/updatable.h"
#include "renderinstancesettings.h"
#include "framerange.h"
#include "CacheHandlers/samples.h"
#include "Sound/esoundsettings.h"
extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
    #include <libswresample/swresample.h>
    #include <libavutil/channel_layout.h>
    #include <libavutil/mathematics.h>
    #include <libavutil/opt.h>
}
class ImageCacheContainer;

class SoundIterator {
public:
    SoundIterator() {}

    bool hasValue() const {
        return !mSamples.isEmpty();
    }

    void fillFrame(AVFrame* const frame) {
        Q_ASSERT(frame->channel_layout == mCurrentSamples->fChannelLayout);
        Q_ASSERT(frame->format == mCurrentSamples->fFormat);
        Q_ASSERT(frame->sample_rate == mCurrentSamples->fSampleRate);
        const int nChannels = static_cast<int>(mCurrentSamples->fNChannels);
        if(mCurrentSamples->fPlanar) {
            for(int i = 0; i < frame->nb_samples; i++) {
                for(int j = 0; j < nChannels; j++) {
                    frame->data[j][i] = mCurrentData[j][mCurrentSample];
                }
                if(mCurrentSample++ >= mEndSample) {
                    if(!next()) return;
                }
            }
        } else {
            for(int i = 0; i < frame->nb_samples; i++) {
                for(int j = 0; j < nChannels; j++) {
                    const int dstId = i*nChannels + j;
                    const int srcId = mCurrentSample*nChannels + j;
                    frame->data[0][dstId] = mCurrentData[0][srcId];
                }
                if(mCurrentSample++ >= mEndSample) {
                    if(!next()) return;
                }
            }
        }
    }

    bool next() {
        if(mSamples.isEmpty()) return false;
        mSamples.removeFirst();
        if(!updateCurrent()) return false;
        return true;
    }

    void add(const stdsptr<Samples>& sound) {
        const bool update = !hasValue();
        mSamples << sound;
        if(update) updateCurrent();
    }

    void clear() {
        mSamples.clear();
        updateCurrent();
    }
private:
    bool updateCurrent() {
        if(mSamples.isEmpty()) {
            mCurrentSamples = nullptr;
            mCurrentData = nullptr;
            mCurrentSample = 0;
            mEndSample = 0;
            return false;
        }
        mCurrentSamples = mSamples.first().get();
        mCurrentData = mCurrentSamples->fData;
        const auto samplesRange = mCurrentSamples->fSampleRange;
        mCurrentSample = 0;
        mEndSample = samplesRange.fMax - samplesRange.fMin;
        return true;
    }

    int mCurrentSample = 0;
    int mEndSample = 0;
    uchar** mCurrentData = nullptr;
    Samples* mCurrentSamples = nullptr;
    QList<stdsptr<Samples>> mSamples;
};

typedef struct OutputStream {
    // pts of the next frame that will be generated
    int64_t fNextPts;

    AVStream *fStream = nullptr;
    AVCodecContext *fCodec = nullptr;
    int fFrameNbSamples = 0;
    AVFrame *fDstFrame = nullptr;
    AVFrame *fSrcFrame = nullptr;
    struct SwsContext *fSwsCtx = nullptr;
    struct SwrContext *fSwrCtx = nullptr;
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
    e_OBJECT
protected:
    VideoEncoder();
public:
    void process();
    void beforeProcessing(const Hardware);
    void afterProcessing();

    bool startNewEncoding(RenderInstanceSettings * const settings) {
        return startEncoding(settings);
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
    void addContainer(const stdsptr<Samples> &cont);

    static VideoEncoder *sInstance;

    static void sInterruptEncoding();
    static bool sStartEncoding(RenderInstanceSettings *settings);
    static void sAddCacheContainerToEncoder(const stdsptr<ImageCacheContainer> &cont);
    static void sAddCacheContainerToEncoder(const stdsptr<Samples> &cont);
    static void sFinishEncoding();
    static bool sEncodingSuccessfulyStarted();

    VideoEncoderEmitter *getEmitter() {
        return &mEmitter;
    }

    bool getCurrentlyEncoding() const {
        return mCurrentlyEncoding;
    }
protected:
    void clearContainers();
    VideoEncoderEmitter mEmitter;
    void interrupEncoding();
    void finishEncodingSuccess();
    void finishEncodingNow();
    bool startEncoding(RenderInstanceSettings * const settings);
    void startEncodingNow();

    bool mEncodingSuccesfull = false;
    bool mEncodingFinished = false;
    bool mInterruptEncoding = false;

    eSoundSettingsData mInSoundSettings;
    OutputStream mVideoStream;
    OutputStream mAudioStream;
    AVFormatContext *mFormatContext = nullptr;
    AVOutputFormat *mOutputFormat = nullptr;
    bool mCurrentlyEncoding = false;
    QList<stdsptr<ImageCacheContainer>> mNextContainers;
    QList<stdsptr<Samples>> mNextSoundConts;

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
