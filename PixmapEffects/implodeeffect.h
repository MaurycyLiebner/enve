#ifndef IMPLODEEFFECT_H
#define IMPLODEEFFECT_H
#include "pixmapeffect.h"

struct ImplodeEffectRenderData : public PixmapEffectRenderData {
    friend class StdSelfRef;

    void applyEffectsSk(const SkBitmap &imgPtr,
                        const fmt_filters::image &img,
                        const qreal &scale);

    qreal factor;
private:
    ImplodeEffectRenderData() {}
};

class ImplodeEffect : public PixmapEffect {
    friend class SelfRef;
public:
    qreal getMargin() { return 0.; }

    PixmapEffectRenderDataSPtr getPixmapEffectRenderDataForRelFrameF(
            const qreal &relFrame, BoundingBoxRenderData*);
protected:
    ImplodeEffect(qreal radius = 10.);
private:
    QrealAnimatorQSPtr mFactorAnimator;
};

#endif // IMPLODEEFFECT_H
