#ifndef REPLACECOLOREFFECT_H
#define REPLACECOLOREFFECT_H
#include "pixmapeffect.h"
#include "Animators/coloranimator.h"

struct ReplaceColorEffectRenderData : public PixmapEffectRenderData {
    friend class StdSelfRef;

    void applyEffectsSk(const SkBitmap &bitmap,
                        const qreal scale);

    int redR;
    int greenR;
    int blueR;
    int alphaR;
    int redT;
    int greenT;
    int blueT;
    int alphaT;
    int tolerance;
    qreal smoothness;
private:
    ReplaceColorEffectRenderData() {}
};

class ReplaceColorEffect : public PixmapEffect {
    friend class SelfRef;
public:
    qreal getMargin() { return 0.; }

    stdsptr<PixmapEffectRenderData> getPixmapEffectRenderDataForRelFrameF(
            const qreal relFrame, BoundingBoxRenderData*);
    void readProperty(QIODevice * const src);
    void writeProperty(QIODevice * const target) const;
protected:
    ReplaceColorEffect();
private:
    qsptr<ColorAnimator> mFromColor;
    qsptr<ColorAnimator> mToColor;

    qsptr<QrealAnimator> mToleranceAnimator;
    qsptr<QrealAnimator> mSmoothnessAnimator;
};

#endif // REPLACECOLOREFFECT_H
