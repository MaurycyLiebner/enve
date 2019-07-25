#ifndef COLORIZEEFFECT_H
#define COLORIZEEFFECT_H
#include "pixmapeffect.h"

struct ColorizeEffectRenderData : public PixmapEffectRenderData {
    friend class StdSelfRef;

    void applyEffectsSk(const SkBitmap &bitmap,
                        const qreal scale);

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
    stdsptr<PixmapEffectRenderData> getPixmapEffectRenderDataForRelFrameF(
            const qreal relFrame, BoxRenderData*);
protected:
    ColorizeEffect();
private:
    qsptr<QrealAnimator> mHueAnimator;
    qsptr<QrealAnimator> mSaturationAnimator;
    qsptr<QrealAnimator> mLightnessAnimator;
    qsptr<QrealAnimator> mAlphaAnimator;
};

#endif // COLORIZEEFFECT_H
