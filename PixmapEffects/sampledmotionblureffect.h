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
    BoundingBoxRenderDataPtr boxData;
    QList<BoundingBoxRenderDataSPtr> samples;
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

    PixmapEffectRenderDataSPtr getPixmapEffectRenderDataForRelFrameF(
            const qreal &relFrame, BoundingBoxRenderData* data);
    void prp_setAbsFrame(const int &frame);
protected:
    SampledMotionBlurEffect(BoundingBox *box = nullptr);
private:
    void getParentBoxFirstLastMarginAjusted(int *firstT, int *lastT,
                                            const int &relFrame);
    BoundingBoxQPtr mParentBox;
    QrealAnimatorQSPtr mOpacity;
    QrealAnimatorQSPtr mNumberSamples;
    QrealAnimatorQSPtr mFrameStep;
};
#endif // SAMPLEDMOTIONBLUREFFECT_H
