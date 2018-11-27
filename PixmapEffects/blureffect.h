#ifndef BLUREFFECT_H
#define BLUREFFECT_H
#include "pixmapeffect.h"

struct BlurEffectRenderData : public PixmapEffectRenderData {
    friend class StdSelfRef;

    void applyEffectsSk(const SkBitmap &imgPtr,
                        const fmt_filters::image &img,
                        const qreal &scale);

    bool hasKeys;
    bool highQuality;
    qreal blurRadius;
protected:
    BlurEffectRenderData() {}
};

class BlurEffect : public PixmapEffect {
    friend class SelfRef;
public:
    qreal getMargin();
    qreal getMarginAtRelFrame(const int &relFrame);


    PixmapEffectRenderDataSPtr getPixmapEffectRenderDataForRelFrameF(
            const qreal &relFrame, BoundingBoxRenderData* );
    void readProperty(QIODevice *target);
    void writeProperty(QIODevice *target);
protected:
    BlurEffect();
private:
    BoolPropertyQSPtr mHighQuality;
    QrealAnimatorQSPtr mBlurRadius;
};

#endif // BLUREFFECT_H
