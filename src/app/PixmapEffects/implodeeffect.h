#ifndef IMPLODEEFFECT_H
#define IMPLODEEFFECT_H
#include "pixmapeffect.h"

struct ImplodeEffectRenderData : public PixmapEffectRenderData {
    friend class StdSelfRef;

    void applyEffectsSk(const SkBitmap &bitmap,
                        const qreal scale);

    qreal factor;
private:
    ImplodeEffectRenderData() {}
};

class ImplodeEffect : public PixmapEffect {
    friend class SelfRef;
public:
    stdsptr<PixmapEffectRenderData> getPixmapEffectRenderDataForRelFrameF(
            const qreal relFrame, BoundingBoxRenderData*);
protected:
    ImplodeEffect();
private:
    qsptr<QrealAnimator> mFactorAnimator;
};

#endif // IMPLODEEFFECT_H
