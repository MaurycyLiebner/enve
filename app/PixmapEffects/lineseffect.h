#ifndef LINESEFFECT_H
#define LINESEFFECT_H
#include "pixmapeffect.h"

struct LinesEffectRenderData : public PixmapEffectRenderData {
    friend class StdSelfRef;

    void applyEffectsSk(const SkBitmap &bitmap,
                        const qreal &scale);

    bool vertical = false;
    qreal linesDistance;
    qreal linesWidth;
protected:
    LinesEffectRenderData() {}
};

class LinesEffect : public PixmapEffect {
    friend class SelfRef;
public:
    qreal getMargin() { return 0.; }

    stdsptr<PixmapEffectRenderData> getPixmapEffectRenderDataForRelFrameF(
            const qreal &relFrame, BoundingBoxRenderData*);
protected:
    LinesEffect(qreal linesWidth = 5.,
                qreal linesDistance = 5.);
private:
    bool mVertical = false;
    qsptr<QrealAnimator> mLinesDistance;
    qsptr<QrealAnimator> mLinesWidth;
};
#endif // LINESEFFECT_H
