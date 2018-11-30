#ifndef COLORIZEEFFECT_H
#define COLORIZEEFFECT_H
#include "pixmapeffect.h"

struct ColorizeEffectRenderData : public PixmapEffectRenderData {
    friend class StdSelfRef;

    void applyEffectsSk(const SkBitmap &imgPtr,
                        const fmt_filters::image &img,
                        const qreal &scale);

    qreal hue;
    qreal saturation;
    qreal lightness;
    qreal alpha;
protected:
    ColorizeEffectRenderData() {}
};

class ColorizeEffect : public PixmapEffect {
    friend class SelfRef;
public:
    qreal getMargin() { return 0.; }

    PixmapEffectRenderDataSPtr getPixmapEffectRenderDataForRelFrameF(
            const qreal &relFrame, BoundingBoxRenderData*);
    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);
protected:
    ColorizeEffect();
private:
    QrealAnimatorQSPtr mHueAnimator;
    QrealAnimatorQSPtr mSaturationAnimator;
    QrealAnimatorQSPtr mLightnessAnimator;
    QrealAnimatorQSPtr mAlphaAnimator;
};

#endif // COLORIZEEFFECT_H
