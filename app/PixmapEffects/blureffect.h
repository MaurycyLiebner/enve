#ifndef BLUREFFECT_H
#define BLUREFFECT_H
#include "pixmapeffect.h"
class BoolProperty;

struct BlurEffectRenderData : public PixmapEffectRenderData {
    friend class StdSelfRef;
protected:
    BlurEffectRenderData() {}
public:
    void applyEffectsSk(const SkBitmap &bitmap,
                        const qreal &scale);

    qreal fBlurRadius;
};

class BlurEffect : public PixmapEffect {
    friend class SelfRef;
public:
    qreal getMargin();
    qreal getMarginAtRelFrame(const int &relFrame);


    stdsptr<PixmapEffectRenderData> getPixmapEffectRenderDataForRelFrameF(
            const qreal &relFrame, BoundingBoxRenderData* );
    void readProperty(QIODevice *target);
    void writeProperty(QIODevice * const target) const;
protected:
    BlurEffect();
private:
    qsptr<QrealAnimator> mBlurRadius;
};

#endif // BLUREFFECT_H
