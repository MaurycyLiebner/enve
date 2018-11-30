#ifndef BRIGHTNESSEFFECT_H
#define BRIGHTNESSEFFECT_H
#include "pixmapeffect.h"

struct BrightnessEffectRenderData : public PixmapEffectRenderData {
    friend class StdSelfRef;
    void applyEffectsSk(const SkBitmap &imgPtr,
                        const fmt_filters::image &img,
                        const qreal &scale);

    bool hasKeys = false;
    qreal brightness;
private:
    BrightnessEffectRenderData() {}
};

class BrightnessEffect : public PixmapEffect {
    friend class SelfRef;
public:
    qreal getMargin() { return 0.; }

    PixmapEffectRenderDataSPtr getPixmapEffectRenderDataForRelFrameF(
            const qreal &relFrame, BoundingBoxRenderData*);
    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);
protected:
    BrightnessEffect(qreal brightness = .0);
private:
    QrealAnimatorQSPtr mBrightnessAnimator;
};
#endif // BRIGHTNESSEFFECT_H
