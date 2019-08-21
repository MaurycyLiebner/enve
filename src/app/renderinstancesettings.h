#ifndef RENDERINSTANCESETTINGS_H
#define RENDERINSTANCESETTINGS_H
class Canvas;
#include "outputsettings.h"
#include "smartPointers/ememory.h"
#include "esettings.h"

struct RenderSettings {
    qreal fResolution = 1;
    qreal fFps = 24;
    AVRational fTimeBase = { 1, 24 }; // inverse of fps - 1/fps
    int fVideoWidth = 0;
    int fVideoHeight = 0;

    int fMinFrame = 0;
    int fMaxFrame = 0;
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
    void setCurrentRenderFrame(const int currentRenderFrame);
    int currentRenderFrame();
    const OutputSettings &getOutputRenderSettings();
    void setOutputRenderSettings(const OutputSettings &settings);
    const RenderSettings &getRenderSettings();
    void setRenderSettings(const RenderSettings &settings);
    void renderingAboutToStart();
    void setCurrentState(const RenderState &state,
                         const QString &text = "");
    const QString &getRenderError() const;
    RenderState getCurrentState() const;
    void copySettingsFromOutputSettingsProfile();
    void setOutputSettingsProfile(OutputSettingsProfile *profile);
    OutputSettingsProfile *getOutputSettingsProfile();
signals:
    void stateChanged(const RenderState state);
    void renderFrameChanged(const int frame);
private:
    RenderState mState = NONE;
    int mCurrentRenderFrame = 0;

    QString mOutputDestination;
    QString mRenderError;

    stdptr<OutputSettingsProfile> mOutputSettingsProfile;

    qptr<Canvas> mTargetCanvas;

    RenderSettings mRenderSettings;
    OutputSettings mOutputSettings;
};

#endif // RENDERINSTANCESETTINGS_H
