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

void BlurEffect::apply(const fmt_filters::image &img) {
    qreal radius = mBlurRadius.getCurrentValue();
    fmt_filters::blur(img, radius, radius*0.3333);
}

void BlurEffect::setRadius(qreal radius) {
    mBlurRadius.setCurrentValue(radius);
}
