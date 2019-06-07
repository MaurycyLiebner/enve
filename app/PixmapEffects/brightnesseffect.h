#ifndef BRIGHTNESSEFFECT_H
#define BRIGHTNESSEFFECT_H
#include "pixmapeffect.h"

struct BrightnessEffectRenderData : public PixmapEffectRenderData {
    friend class StdSelfRef;
    void applyEffectsSk(const SkBitmap &bitmap,
                        const qreal scale);

    bool hasKeys = false;
    qreal brightness;
private:
    BrightnessEffectRenderData() {}
};

class BrightnessEffect : public PixmapEffect {
    friend class SelfRef;
public:
    qreal getMargin() { return 0.; }

    stdsptr<PixmapEffectRenderData> getPixmapEffectRenderDataForRelFrameF(
            const qreal relFrame, BoundingBoxRenderData*);
    void writeProperty(QIODevice * const dst) const;
    void readProperty(QIODevice * const src);
protected:
    BrightnessEffect(qreal brightness = .0);
private:
    qsptr<QrealAnimator> mBrightnessAnimator;
};
#endif // BRIGHTNESSEFFECT_H
