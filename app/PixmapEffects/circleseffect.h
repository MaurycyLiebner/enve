#ifndef CIRCLESEFFECT_H
#define CIRCLESEFFECT_H
#include "pixmapeffect.h"

struct CirclesEffectRenderData : public PixmapEffectRenderData {
    friend class StdSelfRef;

    void applyEffectsSk(const SkBitmap &imgPtr,
                        const fmt_filters::image &img,
                        const qreal &scale);

    qreal circlesDistance;
    qreal circlesRadius;
protected:
    CirclesEffectRenderData() {}
};

class CirclesEffect : public PixmapEffect {
    friend class SelfRef;
public:
    qreal getMargin() { return 0.; }
    stdsptr<PixmapEffectRenderData> getPixmapEffectRenderDataForRelFrameF(
            const qreal &relFrame, BoundingBoxRenderData*);
protected:
    CirclesEffect(qreal circlesRadius = 5.,
                  qreal circlesDistance = 5.);
private:
    qsptr<QrealAnimator> mCirclesDistance;
    qsptr<QrealAnimator> mCirclesRadius;
};
#endif // CIRCLESEFFECT_H
