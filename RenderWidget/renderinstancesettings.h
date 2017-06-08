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

    const QString &getOutputDestination() {
        return mOutputDestination;
    }

    void setTargetCanvas(Canvas *canvas) {
        mTargetCanvas = canvas;
    }

    Canvas *getTargetCanvas() {
        return mTargetCanvas;
    }

private:
    Canvas *mTargetCanvas;
    QString mName;

    QString mOutputDestination;
};

#endif // RENDERINSTANCESETTINGS_H
