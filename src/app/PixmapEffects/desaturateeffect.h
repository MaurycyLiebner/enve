#ifndef DESATURATEEFFECT_H
#define DESATURATEEFFECT_H
#include "pixmapeffect.h"

struct DesaturateEffectRenderData : public PixmapEffectRenderData {
    friend class StdSelfRef;

    void applyEffectsSk(const SkBitmap &bitmap,
                        const qreal scale);

    qreal influence;
protected:
    DesaturateEffectRenderData() {}
};

class DesaturateEffect : public PixmapEffect {
    friend class SelfRef;
protected:
    DesaturateEffect();
public:
    stdsptr<PixmapEffectRenderData> getPixmapEffectRenderDataForRelFrameF(
            const qreal relFrame, BoxRenderData*);
private:
    qsptr<QrealAnimator> mInfluenceAnimator;
};
#endif // DESATURATEEFFECT_H
