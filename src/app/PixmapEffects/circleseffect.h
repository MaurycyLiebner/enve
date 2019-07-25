#ifndef CIRCLESEFFECT_H
#define CIRCLESEFFECT_H
#include "pixmapeffect.h"

struct CirclesEffectRenderData : public PixmapEffectRenderData {
    friend class StdSelfRef;

    void applyEffectsSk(const SkBitmap &bitmap,
                        const qreal scale);

    qreal circlesDistance;
    qreal circlesRadius;
protected:
    CirclesEffectRenderData() {}
};

class CirclesEffect : public PixmapEffect {
    friend class SelfRef;
public:
    stdsptr<PixmapEffectRenderData> getPixmapEffectRenderDataForRelFrameF(
            const qreal relFrame, BoxRenderData*);
protected:
    CirclesEffect(qreal circlesRadius = 5.,
                  qreal circlesDistance = 5.);
private:
    qsptr<QrealAnimator> mCirclesDistance;
    qsptr<QrealAnimator> mCirclesRadius;
};
#endif // CIRCLESEFFECT_H
