#ifndef SWIRLEFFECT_H
#define SWIRLEFFECT_H
#include "pixmapeffect.h"

struct SwirlEffectRenderData : public PixmapEffectRenderData {
    friend class StdSelfRef;

    void applyEffectsSk(const SkBitmap &bitmap,
                        const qreal scale);

    qreal degrees;
protected:
    SwirlEffectRenderData() {}
};

class SwirlEffect : public PixmapEffect {
    friend class SelfRef;
public:
    qreal getMargin() { return 0.; }

    stdsptr<PixmapEffectRenderData> getPixmapEffectRenderDataForRelFrameF(
            const qreal relFrame, BoundingBoxRenderData*);
protected:
    SwirlEffect(qreal degrees = 45.);
private:
    qsptr<QrealAnimator> mDegreesAnimator;
};

#endif // SWIRLEFFECT_H
