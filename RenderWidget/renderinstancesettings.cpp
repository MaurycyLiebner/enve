#include "renderinstancesettings.h"
#include "canvas.h"

RenderInstanceSettings::RenderInstanceSettings()
{

}

qreal RenderInstanceSettings::getFps() const {
    return 24.;
    //return mTargetCanvas->getFps();
}

int RenderInstanceSettings::getVideoWidth() const {
    return 1920;
    //return mTargetCanvas->getCanvasWidth();
}

int RenderInstanceSettings::getVideoHeight() const {
    return 1080;
    //return mTargetCanvas->getCanvasHeight();
}
