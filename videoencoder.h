#ifndef VIDEOENCODER_H
#define VIDEOENCODER_H
#include <QString>
#include <QList>
#include "skiaincludes.h"
#include "updatable.h"
#include "RenderWidget/renderinstancesettings.h"
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

typedef struct OutputStream {
    AVStream *st;
    AVCodecContext *enc;

    /* pts of the next frame that will be generated */
    int64_t next_pts;

    AVFrame *frame;
    AVFrame *tmp_frame;

    float t, tincr, tincr2;

    struct SwsContext *sws_ctx;
    AVAudioResampleContext *avr;
} OutputStream;

class VideoEncoder : public Updatable {
public:
    VideoEncoder();

    void startNewEncoding(const RenderInstanceSettings &settings) {
        if(mStartedEncoding) {
            interruptCurrentEncoding();
            if(!mStartedEncoding) {
                startEncoding(settings);
                return;
            }
            mNewEncodingPlanned = true;
            mWaitingRenderInstanceSettings = settings;
        } else {
            startEncoding(settings);
        }
    }

    void interruptCurrentEncoding() {
        if(!mBeingProcessed && !mSchedulerAdded && !mAwaitingUpdate) {
            finishEncoding();
        } else {
            mEncodingInterrupted = true;
        }
    }

    void finishCurrentEncoding() {
        if(!mBeingProcessed && !mSchedulerAdded && !mAwaitingUpdate) {
            finishEncoding();
        } else {
            mEncodingFinished = true;
        }
    }

    void addImage(const sk_sp<SkImage> &img);
    void processUpdate();
    void beforeUpdate();
    void afterUpdate();

    static VideoEncoder *mVideoEncoderInstance;
    static void interruptEncodingStatic();
    static void startEncodingStatic(const RenderInstanceSettings &settings);
    static void addImageToEncoderStatic(const sk_sp<SkImage> &img);
    static void finishEncodingStatic();
protected:
    void finishEncoding();
    void startEncoding(const RenderInstanceSettings &settings);

    bool mEncodingFinished = false;
    bool mNewEncodingPlanned = false;
    RenderInstanceSettings mWaitingRenderInstanceSettings;
    bool mEncodingInterrupted = false;

    OutputStream mVideoStream;
    OutputStream mAudioStream;
    AVFormatContext *mFormatContext = NULL;
    AVOutputFormat *mOutputFormat = NULL;
    bool mStartedEncoding = false;
    QList<sk_sp<SkImage> > mImages;
    RenderInstanceSettings mRenderInstanceSettings;
    QByteArray mPathByteArray;
    int mHaveVideo = 0;
    int mHaveAudio = 0;
    int mEncodeVideo = 0;
    int mEncodeAudio = 0;

    QList<sk_sp<SkImage> > mUpdateImages;
};

#endif // VIDEOENCODER_H
