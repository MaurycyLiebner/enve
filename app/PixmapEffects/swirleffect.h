#ifndef SWIRLEFFECT_H
#define SWIRLEFFECT_H
#include "pixmapeffect.h"

struct SwirlEffectRenderData : public PixmapEffectRenderData {
    friend class StdSelfRef;

    void applyEffectsSk(const SkBitmap &imgPtr,
                        const fmt_filters::image &img,
                        const qreal &scale);

    qreal degrees;
protected:
    SwirlEffectRenderData() {}
};

class SwirlEffect : public PixmapEffect {
    friend class SelfRef;
public:
    qreal getMargin() { return 0.; }

    PixmapEffectRenderDataSPtr getPixmapEffectRenderDataForRelFrameF(
            const qreal &relFrame, BoundingBoxRenderData*);
protected:
    SwirlEffect(qreal degrees = 45.);
private:
    QrealAnimatorQSPtr mDegreesAnimator;
};

#endif // SWIRLEFFECT_H
