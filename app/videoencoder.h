#ifndef VIDEOENCODER_H
#define VIDEOENCODER_H
#include <QString>
#include <QList>
#include "skia/skiaincludes.h"
#include "updatable.h"
#include "renderinstancesettings.h"
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
class CacheContainer;

typedef struct OutputStream {
    AVStream *st = nullptr;
    AVCodecContext *enc = nullptr;

    /* pts of the next frame that will be generated */
    int64_t next_pts;

    AVFrame *frame = nullptr;
    AVFrame *tmp_frame = nullptr;

    float t, tincr, tincr2;

    struct SwsContext *sws_ctx = nullptr;
    AVAudioResampleContext *avr = nullptr;
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

class VideoEncoder : public _ScheduledTask {
public:
    VideoEncoder();

    void startNewEncoding(RenderInstanceSettings *settings) {
        if(!mCurrentlyEncoding) {
            startEncoding(settings);
        }
    }

    void interruptCurrentEncoding() {
        if(!mBeingProcessed && !mTaskScheduled && !mTaskQued) {
            interrupEncoding();
        } else {
            mInterruptEncoding = true;
        }
    }

    void finishCurrentEncoding() {
        if(!mBeingProcessed && !mTaskScheduled && !mTaskQued) {
            finishEncodingSuccess();
        } else {
            mEncodingFinished = true;
        }
    }

    void addContainer(CacheContainer *cont);
    void _processUpdate();
    void beforeProcessingStarted();
    void afterProcessingFinished();

    static VideoEncoder *mVideoEncoderInstance;
    static VideoEncoderEmitter *getVideoEncoderEmitter();

    static void interruptEncodingStatic();
    static void startEncodingStatic(RenderInstanceSettings *settings);
    static void addCacheContainerToEncoderStatic(CacheContainer *cont);
    static void finishEncodingStatic();
    static bool encodingSuccessfulyStartedStatic();

    bool shouldUpdate() { return !mTaskQued && mCurrentlyEncoding; }

    VideoEncoderEmitter *getEmitter() {
        return &mEmitter;
    }

    bool getCurrentlyEncoding() {
        return mCurrentlyEncoding;
    }
protected:
    void clearContainers();
    VideoEncoderEmitter mEmitter;
    void interrupEncoding();
    void finishEncodingSuccess();
    void finishEncodingNow();
    void startEncoding(RenderInstanceSettings *settings);
    bool startEncodingNow(QString &error);

    bool mEncodingSuccesfull = false;
    bool mEncodingFinished = false;
    bool mInterruptEncoding = false;

    OutputStream mVideoStream;
    OutputStream mAudioStream;
    AVFormatContext *mFormatContext = nullptr;
    AVOutputFormat *mOutputFormat = nullptr;
    bool mCurrentlyEncoding = false;
    QList<stdsptr<CacheContainer> > mNextContainers;

    RenderSettings mRenderSettings;
    OutputSettings mOutputSettings;
    RenderInstanceSettings *mRenderInstanceSettings = nullptr;
    QByteArray mPathByteArray;
    bool mHaveVideo = 0;
    bool mHaveAudio = 0;
    int mEncodeVideo = 0;
    int mEncodeAudio = 0;

    QString mUpdateError;
    bool mUpdateFailed = false;
    int _mCurrentContainerId = 0;
    int _mCurrentContainerFrame = 0; // some containers will add multiple frames

    QList<stdsptr<CacheContainer> > _mContainers;
};

#endif // VIDEOENCODER_H
