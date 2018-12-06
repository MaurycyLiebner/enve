#ifndef OILEFFECT_H
#define OILEFFECT_H
#include "pixmapeffect.h"

struct OilEffectRenderData : public PixmapEffectRenderData {
    friend class StdSelfRef;

    void applyEffectsSk(const SkBitmap &bitmap,
                        const qreal &scale);

    qreal radius;
private:
    OilEffectRenderData() {}
};

class OilEffect : public PixmapEffect {
    friend class SelfRef;
public:
    qreal getMargin() { return 0.; }

    stdsptr<PixmapEffectRenderData> getPixmapEffectRenderDataForRelFrameF(
            const qreal &relFrame, BoundingBoxRenderData*);
protected:
    OilEffect(qreal radius = 2.);
private:
    qsptr<QrealAnimator> mRadiusAnimator;
};

#endif // OILEFFECT_H
