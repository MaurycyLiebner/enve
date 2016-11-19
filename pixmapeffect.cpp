#include "pixmapeffect.h"

PixmapEffect::PixmapEffect() : ComplexAnimator()
{

}

BlurEffect::BlurEffect(qreal radius) {
    mBlurRadius.setCurrentValue(radius);
    setName("blur");
    mBlurRadius.setName("radius");
    mBlurRadius.blockPointer();
    addChildAnimator(&mBlurRadius);
}

void BlurEffect::apply(const fmt_filters::image &img, qreal scale) {
    qreal radius = mBlurRadius.getCurrentValue()*scale;
    fmt_filters::blur(img, radius, radius*0.3333);
}

void BlurEffect::setRadius(qreal radius) {
    mBlurRadius.setCurrentValue(radius);
}

qreal BlurEffect::getMargin()
{
    return mBlurRadius.getCurrentValue();
}
