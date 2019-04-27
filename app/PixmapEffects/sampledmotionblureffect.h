#ifndef SAMPLEDMOTIONBLUREFFECT_H
#define SAMPLEDMOTIONBLUREFFECT_H
#include "pixmapeffect.h"
class BoundingBox;

struct SampledMotionBlurEffectRenderData : public PixmapEffectRenderData {
    friend class StdSelfRef;
    void applyEffectsSk(const SkBitmap &bitmap,
                        const qreal &scale);

    qreal numberSamples;
    qreal opacity;
    stdptr<BoundingBoxRenderData>boxData;
    QList<stdsptr<BoundingBoxRenderData>> samples;
private:
    SampledMotionBlurEffectRenderData() {}
};

class SampledMotionBlurEffect : public PixmapEffect {
    friend class SelfRef;
public:
    qreal getMargin() { return 0; }

    void writeProperty(QIODevice * const target) const;
    void readProperty(QIODevice *target);

    void setParentBox(BoundingBox *box) {
        mParentBox = box;
    }

    FrameRange prp_getIdenticalRelRange(const int &relFrame) const;

    stdsptr<PixmapEffectRenderData> getPixmapEffectRenderDataForRelFrameF(
            const qreal &relFrame, BoundingBoxRenderData * const data);
protected:
    SampledMotionBlurEffect(BoundingBox *box = nullptr);
private:
    FrameRange getParentBoxFirstLastMarginAjusted(const int &relFrame) const;
    qptr<BoundingBox> mParentBox;
    qsptr<QrealAnimator> mOpacity;
    qsptr<QrealAnimator> mNumberSamples;
    qsptr<QrealAnimator> mFrameStep;
};
#endif // SAMPLEDMOTIONBLUREFFECT_H
