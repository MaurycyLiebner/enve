#ifndef RENDERINSTANCESETTINGS_H
#define RENDERINSTANCESETTINGS_H
#include <QString>
class Canvas;

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
        return mBitrate;
    }
private:
    int mBitrate = 400000;
    int mMinFrame = 0;
    int mMaxFrame = 0;
    int mCurrentRenderFrame = 0;
    Canvas *mTargetCanvas;
    QString mName;

    QString mOutputDestination;
};

#endif // RENDERINSTANCESETTINGS_H
