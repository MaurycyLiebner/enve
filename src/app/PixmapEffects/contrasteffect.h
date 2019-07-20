#ifndef CONTRASTEFFECT_H
#define CONTRASTEFFECT_H
#include "pixmapeffect.h"

struct ContrastEffectRenderData : public PixmapEffectRenderData {
    friend class StdSelfRef;

    void applyEffectsSk(const SkBitmap &bitmap,
                        const qreal scale);

    bool hasKeys = false;
    qreal contrast;
private:
    ContrastEffectRenderData() {}
};

class ContrastEffect : public PixmapEffect {
    friend class SelfRef;
protected:
    ContrastEffect(const qreal contrast = 0);
public:
    stdsptr<PixmapEffectRenderData> getPixmapEffectRenderDataForRelFrameF(
            const qreal relFrame, BoundingBoxRenderData*);
private:
    qsptr<QrealAnimator> mContrastAnimator;
};

#endif // CONTRASTEFFECT_H
