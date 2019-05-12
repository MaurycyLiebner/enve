#ifndef VIDEOENCODER_H
#define VIDEOENCODER_H
#include <QString>
#include <QList>
#include "skia/skiaincludes.h"
#include "updatable.h"
#include "renderinstancesettings.h"
#include "framerange.h"
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

typedef struct OutputStream {
    AVStream *fStream = nullptr;
    AVCodecContext *fCodec = nullptr;

    /* pts of the next frame that will be generated */
    int64_t fNextPts;

    AVFrame *fFrame = nullptr;
    AVFrame *fTmpFrame = nullptr;

    float fT, fTincr, fTincr2;

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
    bool mHaveVideo = 0;
    bool mHaveAudio = 0;
    bool mEncodeVideo = 0;
    bool mEncodeAudio = 0;

    int _mCurrentContainerId = 0;
    int _mCurrentContainerFrame = 0; // some containers will add multiple frames
    FrameRange _mRenderRange;

    QList<stdsptr<ImageCacheContainer>> _mContainers;
};

#endif // VIDEOENCODER_H
