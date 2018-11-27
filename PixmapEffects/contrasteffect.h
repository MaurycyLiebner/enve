#ifndef CONTRASTEFFECT_H
#define CONTRASTEFFECT_H
#include "pixmapeffect.h"

struct ContrastEffectRenderData : public PixmapEffectRenderData {
    friend class StdSelfRef;

    void applyEffectsSk(const SkBitmap &imgPtr,
                        const fmt_filters::image &img,
                        const qreal &scale);

    bool hasKeys = false;
    qreal contrast;
private:
    ContrastEffectRenderData() {}
};

class ContrastEffect : public PixmapEffect {
    friend class SelfRef;
public:
    qreal getMargin() { return 0.; }

    PixmapEffectRenderDataSPtr getPixmapEffectRenderDataForRelFrameF(
            const qreal &relFrame, BoundingBoxRenderData*);
    void writeProperty(QIODevice *target);
    void readProperty(QIODevice *target);
protected:
    ContrastEffect(qreal contrast = .0);
private:
    QrealAnimatorQSPtr mContrastAnimator;
};

#endif // CONTRASTEFFECT_H
