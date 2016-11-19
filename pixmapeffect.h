#ifndef PIXMAPEFFECT_H
#define PIXMAPEFFECT_H
#include "fmt_filters.h"
#include "complexanimator.h"

class PixmapEffect : public ComplexAnimator
{
public:
    PixmapEffect();
    virtual void apply(const fmt_filters::image &img, qreal scale = 1.) {}
    virtual qreal getMargin() {}
};

class BlurEffect : public PixmapEffect
{
public:
    BlurEffect(qreal radius = 0.);

    void apply(const fmt_filters::image &img, qreal scale);

    void setRadius(qreal radius);

    qreal getMargin();
private:
    QrealAnimator mBlurRadius;
};

#endif // PIXMAPEFFECT_H
