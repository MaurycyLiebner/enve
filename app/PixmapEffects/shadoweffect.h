#ifndef SHADOWEFFECT_H
#define SHADOWEFFECT_H
#include "pixmapeffect.h"
#include "Animators/qpointfanimator.h"
#include "Animators/coloranimator.h"
#include "Properties/boolproperty.h"

struct ShadowEffectRenderData : public PixmapEffectRenderData {
    friend class StdSelfRef;

    void applyEffectsSk(const SkBitmap &imgPtr,
                        const fmt_filters::image &img,
                        const qreal &scale);

    bool hasKeys;
    bool highQuality;
    qreal blurRadius;
    qreal opacity;
    QColor color;
    QPointF translation;
protected:
    ShadowEffectRenderData() {}
};

class ShadowEffect : public PixmapEffect {
    friend class SelfRef;
public:
    qreal getMargin();
    qreal getMarginAtRelFrame(const int &relFrame);

    stdsptr<PixmapEffectRenderData> getPixmapEffectRenderDataForRelFrameF(
            const qreal &relFrame, BoundingBoxRenderData*);
    void readProperty(QIODevice *target);
    void writeProperty(QIODevice *target);
protected:
    ShadowEffect(qreal radius = 10.);
private:
//    QrealAnimator mScale;
    qsptr<BoolProperty> mHighQuality;
    qsptr<QrealAnimator> mBlurRadius;
    qsptr<QrealAnimator> mOpacity;
    qsptr<ColorAnimator> mColor;
    qsptr<QPointFAnimator> mTranslation;
};

#endif // SHADOWEFFECT_H
