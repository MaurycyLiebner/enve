#ifndef RENDERINSTANCESETTINGS_H
#define RENDERINSTANCESETTINGS_H
#include <QString>
#include "selfref.h"
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
#include "sharedpointerdefs.h"
class RenderInstanceWidget;

struct RenderSettings {
    qreal resolution = 1.;
    qreal fps = 24.;
    AVRational timeBase = { 1, 24 }; // inverse of fps - 1/fps
    int videoWidth = 0;
    int videoHeight = 0;

    int minFrame = 0;
    int maxFrame = 0;
};

struct OutputSettings {
    static const QStringList SAMPLE_FORMATS_NAMES;
    static QString getChannelsLayoutNameStatic(const uint64_t &layout);

    AVOutputFormat *outputFormat = nullptr;

    bool videoEnabled = false;
    bool audioEnabled = false;

    AVPixelFormat videoPixelFormat = AV_PIX_FMT_NONE;
    AVSampleFormat audioSampleFormat = AV_SAMPLE_FMT_NONE;

    int videoBitrate = 0;
    int audioSampleRate = 0;
    int audioBitrate = 0;

    uint64_t audioChannelsLayout = 0;

    AVCodec *videoCodec = nullptr;
    AVCodec *audioCodec = nullptr;
};

class OutputSettingsProfile : public SelfRef {
public:
    OutputSettingsProfile() {}

    const QString &getName() const {
        return mName;
    }

    void setName(const QString &name) {
        mName = name;
    }

    const OutputSettings &getSettings() const {
        return mSettings;
    }

    void setSettings(const OutputSettings &settings) {
        mSettings = settings;
    }
protected:
    QString mName = "Untitled";
    OutputSettings mSettings;
};

class RenderInstanceSettings {
public:
    enum RenderState {
        NONE,
        ERROR,
        FINISHED,
        RENDERING,
        PAUSED,
        WAITING
    };
    RenderInstanceSettings(const CanvasQSPtr& canvas);

    const QString &getName();

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

    const int &currentRenderFrame() {
        return mCurrentRenderFrame;
    }

    const OutputSettings &getOutputRenderSettings() {
        return mOutputSettings;
    }

    void setOutputRenderSettings(const OutputSettings &settings) {
        mOutputSettings = settings;
    }

    const RenderSettings &getRenderSettings() {
        return mRenderSettings;
    }

    void setRenderSettings(const RenderSettings &settings) {
        mRenderSettings = settings;
    }

    void renderingAboutToStart();

    void setCurrentState(const RenderState &state,
                         const QString &text = "");

    const QString &getRenderError() const {
        return mRenderError;
    }

    const RenderState &getCurrentState() const {
        return mState;
    }

    void setParentWidget(RenderInstanceWidget *wid) {
        mParentWidget = wid;
    }

    void copySettingsFromOutputSettingsProfile() {
        OutputSettingsProfile *profileT = mOutputSettingsProfile.data();
        if(profileT == nullptr) return;
        mOutputSettings = profileT->getSettings();
    }

    void setOutputSettingsProfile(OutputSettingsProfile *profile) {
        if(profile == nullptr) {
            mOutputSettingsProfile.clear();
        } else {
            mOutputSettingsProfile = profile;
        }
        copySettingsFromOutputSettingsProfile();
    }

    OutputSettingsProfile *getOutputSettingsProfile() {
        return mOutputSettingsProfile;
    }
private:
    void updateParentWidget();
    RenderState mState = NONE;
    int mCurrentRenderFrame = 0;

    OutputSettingsProfileQPtr mOutputSettingsProfile;
    RenderInstanceWidget *mParentWidget = nullptr;

    Canvas *mTargetCanvas;

    RenderSettings mRenderSettings;
    OutputSettings mOutputSettings;

    QString mOutputDestination;

    QString mRenderError;
};

#endif // RENDERINSTANCESETTINGS_H
