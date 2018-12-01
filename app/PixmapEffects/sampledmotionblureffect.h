#ifndef SAMPLEDMOTIONBLUREFFECT_H
#define SAMPLEDMOTIONBLUREFFECT_H
#include "pixmapeffect.h"

struct SampledMotionBlurEffectRenderData : public PixmapEffectRenderData {
    friend class StdSelfRef;
    void applyEffectsSk(const SkBitmap &imgPtr,
                        const fmt_filters::image &img,
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
    qreal getMargin() { return 0.; }

    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);

    void setParentBox(BoundingBox *box) {
        mParentBox = box;
    }

    void prp_getFirstAndLastIdenticalRelFrame(int *firstIdentical,
                                              int *lastIdentical,
                                              const int &relFrame);

    stdsptr<PixmapEffectRenderData> getPixmapEffectRenderDataForRelFrameF(
            const qreal &relFrame, BoundingBoxRenderData* data);
    void prp_setAbsFrame(const int &frame);
protected:
    SampledMotionBlurEffect(BoundingBox *box = nullptr);
private:
    void getParentBoxFirstLastMarginAjusted(int *firstT, int *lastT,
                                            const int &relFrame);
    qptr<BoundingBox> mParentBox;
    qsptr<QrealAnimator> mOpacity;
    qsptr<QrealAnimator> mNumberSamples;
    qsptr<QrealAnimator> mFrameStep;
};
#endif // SAMPLEDMOTIONBLUREFFECT_H
