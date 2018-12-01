#ifndef DESATURATEEFFECT_H
#define DESATURATEEFFECT_H
#include "pixmapeffect.h"

struct DesaturateEffectRenderData : public PixmapEffectRenderData {
    friend class StdSelfRef;

    void applyEffectsSk(const SkBitmap &imgPtr,
                        const fmt_filters::image &img,
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
    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);
protected:
    DesaturateEffect(qreal influence = .5);
private:
    qsptr<QrealAnimator> mInfluenceAnimator;
};
#endif // DESATURATEEFFECT_H
