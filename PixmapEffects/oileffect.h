#ifndef OILEFFECT_H
#define OILEFFECT_H
#include "pixmapeffect.h"

struct OilEffectRenderData : public PixmapEffectRenderData {
    friend class StdSelfRef;

    void applyEffectsSk(const SkBitmap &imgPtr,
                        const fmt_filters::image &img,
                        const qreal &scale);

    qreal radius;
private:
    OilEffectRenderData() {}
};

class OilEffect : public PixmapEffect {
    friend class SelfRef;
public:
    qreal getMargin() { return 0.; }

    PixmapEffectRenderDataSPtr getPixmapEffectRenderDataForRelFrameF(
            const qreal &relFrame, BoundingBoxRenderData*);
protected:
    OilEffect(qreal radius = 2.);
private:
    QrealAnimatorQSPtr mRadiusAnimator;
};

#endif // OILEFFECT_H
