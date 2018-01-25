#ifndef RENDERINSTANCESETTINGS_H
#define RENDERINSTANCESETTINGS_H
#include <QString>
class Canvas;
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


class RenderInstanceSettings {
public:
    RenderInstanceSettings();

    const QString &getName() {
        return mName;
    }

    void setName(const QString &name) {
        mName = name;
    }

    void setOutputDestination(const QString &outputDestination) {
        mOutputDestination = outputDestination;
    }

    const QString &getOutputDestination() const {
        return mOutputDestination;
    }

    void setTargetCanvas(Canvas *canvas) {
        mTargetCanvas = canvas;
    }

    Canvas *getTargetCanvas() {
        return mTargetCanvas;
    }

    void setCurrentRenderFrame(const int &currentRenderFrame) {
        mCurrentRenderFrame = currentRenderFrame;
    }

    const int &minFrame() const {
        return mMinFrame;
    }

    const int &maxFrame() const {
        return mMaxFrame;
    }

    void setMaxFrame(const int &maxFrameT) {
        mMaxFrame = maxFrameT;
    }

    void setMinFrame(const int &minFrameT) {
        mMinFrame = minFrameT;
    }

    const int &currentRenderFrame() {
        return mCurrentRenderFrame;
    }

    qreal getFps() const;

    int getVideoWidth() const;
    int getVideoHeight() const;
    const int &getVideoBitrate() const {
        return mVideoBitrate;
    }

    void setVideoBitrate(const int &bitrate) {
        mVideoBitrate = bitrate;
    }

    AVCodec *getVideoCodec() const {
        return mVideoCodec;
    }

    void setVideoCodec(AVCodec *codec) {
        mVideoCodec = codec;
    }

    AVCodec *getAudioCodec() const {
        return mAudioCodec;
    }

    void setAudioCodec(AVCodec *codec) {
        mAudioCodec = codec;
    }

    void setVideoPixelFormat(const AVPixelFormat &format) {
        mVideoPixelFormat = format;
    }

    const AVPixelFormat &getVideoPixelFormat() const {
        return mVideoPixelFormat;
    }

    AVOutputFormat *getOutputFormat() const {
        return mOutputFormat;
    }

    void setOutputFormat(AVOutputFormat *format) {
        mOutputFormat = format;
    }

    int getAudioSampleRate() const {
        return mAudioSampleRate;
    }

    void setAudioSampleRate(const int &sampleRate) {
        mAudioSampleRate = sampleRate;
    }

    AVSampleFormat getAudioSampleFormat() const {
        return mAudioSampleFormat;
    }

    void setAudioSampleFormat(const AVSampleFormat &format) {
        mAudioSampleFormat = format;
    }

    void setVideoEnabled(const bool &enabled) {
        mVideoEnabled = enabled;
    }

    bool getVideoEnabled() const {
        return mVideoEnabled;
    }

    void setAudioEnabled(const bool &enabled) {
        mAudioEnabled = enabled;
    }

    bool getAudioEnabled() const {
        return mAudioEnabled;
    }

    void updateRenderVars();

    AVRational getTimeBase() const {
        return mTimeBase;
    }

    int getAudioBitrate() const {
        return mAudioBitrate;
    }

    void setAudioBitrate(const int &bitrate) {
        mAudioBitrate = bitrate;
    }

    uint64_t getAudioChannelsLayout() const {
        return mAudioChannelsLayout;
    }

    void setAudioChannelsLayout(const uint64_t &layout) {
        mAudioChannelsLayout = layout;
    }
private:
    qreal mFps = 24.;
    AVRational mTimeBase = { 1, 24 }; // inverse of fps - 1/fps
    int mVideoWidth = 0;
    int mVideoHeight = 0;

    int mMinFrame = 0;
    int mMaxFrame = 0;
    int mCurrentRenderFrame = 0;

    Canvas *mTargetCanvas;
    QString mName;

    AVOutputFormat *mOutputFormat = NULL;

    bool mVideoEnabled = true;
    AVCodec *mVideoCodec = NULL;
    AVPixelFormat mVideoPixelFormat = AV_PIX_FMT_NONE;
    int mVideoBitrate = 0;

    bool mAudioEnabled = false;
    AVCodec *mAudioCodec = NULL;
    int mAudioSampleRate = 0;
    int mAudioBitrate = 0;
    AVSampleFormat mAudioSampleFormat = AV_SAMPLE_FMT_NONE;
    uint64_t mAudioChannelsLayout = 0;

    QString mOutputDestination;
};

#endif // RENDERINSTANCESETTINGS_H
