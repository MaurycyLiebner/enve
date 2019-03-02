#ifndef DESATURATEEFFECT_H
#define DESATURATEEFFECT_H
#include "pixmapeffect.h"

struct DesaturateEffectRenderData : public PixmapEffectRenderData {
    friend class StdSelfRef;

    void applyEffectsSk(const SkBitmap &bitmap,
                        const qreal &scale);

    qreal influence;
protected:
    DesaturateEffectRenderData() {}
};

class DesaturateEffect : public PixmapEffect {
    friend class SelfRef;
public:
    qreal getMargin() { return 0.; }

    stdsptr<PixmapEffectRenderData> getPixmapEffectRenderDataForRelFrameF(
            const qreal &relFrame, BoundingBoxRenderData*);
    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice *target);
protected:
    DesaturateEffect();
private:
    qsptr<QrealAnimator> mInfluenceAnimator;
};
#endif // DESATURATEEFFECT_H
