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

class OutputSettingsProfile : public StdSelfRef {
    friend class StdSelfRef;
public:
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
    OutputSettingsProfile() {}

    QString mName = "Untitled";
    OutputSettings mSettings;
};

class RenderInstanceSettings : public QObject {
    Q_OBJECT
public:
    enum RenderState {
        NONE,
        ERROR,
        FINISHED,
        RENDERING,
        PAUSED,
        WAITING
    };
    RenderInstanceSettings(Canvas* canvas);
    virtual ~RenderInstanceSettings() {}

    const QString &getName();
    void setOutputDestination(const QString &outputDestination);
    const QString &getOutputDestination() const;
    void setTargetCanvas(Canvas *canvas);
    Canvas *getTargetCanvas();
    void setCurrentRenderFrame(const int &currentRenderFrame);
    const int &currentRenderFrame();
    const OutputSettings &getOutputRenderSettings();
    void setOutputRenderSettings(const OutputSettings &settings);
    const RenderSettings &getRenderSettings();
    void setRenderSettings(const RenderSettings &settings);
    void renderingAboutToStart();
    void setCurrentState(const RenderState &state,
                         const QString &text = "");
    const QString &getRenderError() const;
    const RenderState &getCurrentState() const;
    void copySettingsFromOutputSettingsProfile();
    void setOutputSettingsProfile(OutputSettingsProfile *profile);
    OutputSettingsProfile *getOutputSettingsProfile();
signals:
    void stateChanged();
private:
    RenderState mState = NONE;
    int mCurrentRenderFrame = 0;

    QString mOutputDestination;
    QString mRenderError;

    OutputSettingsProfilePtr mOutputSettingsProfile;

    CanvasQPtr mTargetCanvas;

    RenderSettings mRenderSettings;
    OutputSettings mOutputSettings;
};

#endif // RENDERINSTANCESETTINGS_H
